#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <chrono>
#include <sycl/sycl.hpp>
#include "utils.h"

int main(int argc, char *argv[])
{
  int repeat = 1;

  if (argc != 5) {
    printf("The function converts a dense MxN matrix into a sparse matrix\n");
    printf("The sparse matrix is represented in CSR (Compressed Sparse Row) storage format\n");
    printf("Usage %s <M> <N> <nnz> <repeat>\n", argv[0]);
    printf("nnz is the number of non-zero elements\n");
    return 1;
  }

  int64_t m, n, h_nnz;

  m = atol(argv[1]);
  n = atol(argv[2]);
  h_nnz = atol(argv[3]);
  repeat = atoi(argv[4]);

  // Host problem definition
  const int64_t num_rows = m;
  const int64_t num_cols = n;
  const int64_t dense_size = m * n;

  const int64_t dense_size_bytes  = dense_size * sizeof(float);
  const int64_t value_size_bytes  = h_nnz * sizeof(float);
  const int64_t colidx_size_bytes = h_nnz * sizeof(int64_t);
  const int64_t rowidx_size_bytes = (num_rows + 1) * sizeof(int64_t);

  float *h_dense = (float*) malloc (dense_size_bytes);

  // device results
  float *h_csr_values = (float*) malloc (value_size_bytes);
  int64_t *h_csr_columns = (int64_t*) malloc (colidx_size_bytes);
  int64_t *h_csr_offsets = (int64_t*) malloc (rowidx_size_bytes);

  // reference results
  float *h_csr_values_result = (float*) malloc (value_size_bytes);
  int64_t *h_csr_columns_result = (int64_t*) malloc (colidx_size_bytes);
  int64_t *h_csr_offsets_result = (int64_t*) malloc (rowidx_size_bytes);

  printf("Initializing host matrices..\n");
  init_matrix(h_dense, num_rows, num_cols, h_nnz);
  init_csr(h_csr_offsets_result, h_csr_values_result, h_csr_columns_result,
           h_dense, num_rows, num_cols, h_nnz);

#ifdef USE_GPU
  sycl::queue q(sycl::gpu_selector_v, sycl::property::queue::in_order());
#else
  sycl::queue q(sycl::cpu_selector_v, sycl::property::queue::in_order());
#endif

  // Device memory management
  int64_t *d_csr_offsets = sycl::malloc_device<int64_t>(num_rows + 1, q);
  int64_t *d_csr_columns = sycl::malloc_device<int64_t>(h_nnz, q);
  float *d_csr_values = sycl::malloc_device<float>(h_nnz, q);
  float *d_dense = sycl::malloc_device<float>(dense_size, q);

  q.memcpy(d_dense, h_dense, dense_size_bytes).wait();

  // Kernel to convert dense to sparse CSR format
  sycl::range<1> gws_offset ((num_rows + 255) / 256 * 256);
  sycl::range<1> lws_offset (256);

  q.wait();
  auto start = std::chrono::steady_clock::now();

  for (int iter = 0; iter < repeat; iter++) {
    // Step 1: Count non-zeros per row and compute row offsets
    q.submit([&](sycl::handler &h) {
      h.parallel_for(sycl::nd_range<1>(gws_offset, lws_offset), [=](sycl::nd_item<1> item) {
        int64_t row = item.get_global_id(0);
        if (row < num_rows) {
          int64_t count = 0;
          for (int64_t col = 0; col < num_cols; col++) {
            if (d_dense[row * num_cols + col] != 0.0f) {
              count++;
            }
          }
          d_csr_offsets[row] = count;
        }
        if (row == 0) {
          d_csr_offsets[num_rows] = 0;
        }
      });
    });

    // Step 2: Prefix sum on row offsets
    q.submit([&](sycl::handler &h) {
      h.single_task([=]() {
        int64_t sum = 0;
        for (int64_t i = 0; i < num_rows; i++) {
          int64_t temp = d_csr_offsets[i];
          d_csr_offsets[i] = sum;
          sum += temp;
        }
        d_csr_offsets[num_rows] = sum;
      });
    });

    // Step 3: Fill values and column indices
    q.submit([&](sycl::handler &h) {
      h.parallel_for(sycl::nd_range<1>(gws_offset, lws_offset), [=](sycl::nd_item<1> item) {
        int64_t row = item.get_global_id(0);
        if (row < num_rows) {
          int64_t offset = d_csr_offsets[row];
          int64_t idx = offset;
          for (int64_t col = 0; col < num_cols; col++) {
            float val = d_dense[row * num_cols + col];
            if (val != 0.0f) {
              d_csr_values[idx] = val;
              d_csr_columns[idx] = col;
              idx++;
            }
          }
        }
      });
    });
  }

  q.wait();
  auto end = std::chrono::steady_clock::now();
  auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  printf("Average execution time of DenseToSparse_convert : %f (us)\n", (time * 1e-3f) / repeat);

  // Copy results back
  int64_t nnz = 0;
  q.memcpy(h_csr_offsets, d_csr_offsets, rowidx_size_bytes);
  q.memcpy(&nnz, &d_csr_offsets[num_rows], sizeof(int64_t));
  q.memcpy(h_csr_columns, d_csr_columns, nnz * sizeof(int64_t));
  q.memcpy(h_csr_values, d_csr_values, nnz * sizeof(float));
  q.wait();

  // Verify results
  int correct = 1;
  if (h_nnz != nnz) {
    printf("nnz: %ld != %ld\n", h_nnz, nnz);
    correct = 0;
    goto print_error;
  }

  for (int64_t i = 0; i < num_rows + 1; i++) {
    if (h_csr_offsets[i] != h_csr_offsets_result[i]) {
      printf("rowidx: @%ld %ld != %ld\n", i, h_csr_offsets[i], h_csr_offsets_result[i]);
      correct = 0;
      goto print_error;
    }
  }

  // Column indices may not be sorted
  std::sort(h_csr_columns, h_csr_columns + nnz);
  std::sort(h_csr_values, h_csr_values + nnz);

  std::sort(h_csr_columns_result, h_csr_columns_result + nnz);
  std::sort(h_csr_values_result, h_csr_values_result + nnz);

  for (int64_t i = 0; i < nnz; i++) {
    if (h_csr_columns[i] != h_csr_columns_result[i]) {
      printf("colidx: @%ld %ld != %ld\n", i, h_csr_columns[i], h_csr_columns_result[i]);
      correct = 0;
      goto print_error;
    }
  }
  for (int64_t i = 0; i < nnz; i++) {
    if (h_csr_values[i] != h_csr_values_result[i]) {
      printf("value: @%ld %f != %f\n", i, h_csr_values[i], h_csr_values_result[i]);
      correct = 0;
      goto print_error;
    }
  }

  print_error:
  if (correct)
      printf("dense2sparse_csr_example test PASSED\n");
  else
      printf("dense2sparse_csr_example test FAILED: wrong result\n");

  // Cleanup
  sycl::free(d_csr_offsets, q);
  sycl::free(d_csr_columns, q);
  sycl::free(d_csr_values, q);
  sycl::free(d_dense, q);
  free(h_dense);
  free(h_csr_values);
  free(h_csr_columns);
  free(h_csr_offsets);
  free(h_csr_values_result);
  free(h_csr_columns_result);
  free(h_csr_offsets_result);
  return EXIT_SUCCESS;
}
