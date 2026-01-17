#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <sycl/sycl.hpp>
#include "utils.h"

int main(int argc, char *argv[])
{
  int repeat = 1;

  if (argc != 5) {
    printf("The function converts a sparse matrix in CSR format into a dense matrix\\n");
    printf("Usage %s <M> <N> <nnz> <repeat>\\n", argv[0]);
    printf("nnz is the number of non-zero elements\\n");
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

  // CSR format input
  float *h_csr_values = (float*) malloc (value_size_bytes);
  int64_t *h_csr_columns = (int64_t*) malloc (colidx_size_bytes);
  int64_t *h_csr_offsets = (int64_t*) malloc (rowidx_size_bytes);

  // device result
  float *h_dense_result = (float*) malloc (dense_size_bytes);

  printf("Initializing host matrices..\\n");
  init_matrix(h_dense, num_rows, num_cols, h_nnz);
  init_csr(h_csr_offsets, h_csr_values, h_csr_columns,
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

  q.memcpy(d_csr_offsets, h_csr_offsets, rowidx_size_bytes);
  q.memcpy(d_csr_columns, h_csr_columns, colidx_size_bytes);
  q.memcpy(d_csr_values, h_csr_values, value_size_bytes);
  q.wait();

  // Kernel to convert sparse CSR to dense format
  sycl::range<1> gws_zero ((dense_size + 255) / 256 * 256);
  sycl::range<1> lws_zero (256);

  sycl::range<1> gws_convert ((num_rows + 255) / 256 * 256);
  sycl::range<1> lws_convert (256);

  q.wait();
  auto start = std::chrono::steady_clock::now();

  for (int iter = 0; iter < repeat; iter++) {
    // Step 1: Zero out the dense matrix
    q.submit([&](sycl::handler &h) {
      h.parallel_for(sycl::nd_range<1>(gws_zero, lws_zero), [=](sycl::nd_item<1> item) {
        int64_t idx = item.get_global_id(0);
        if (idx < dense_size) {
          d_dense[idx] = 0.0f;
        }
      });
    });

    // Step 2: Convert CSR to dense
    q.submit([&](sycl::handler &h) {
      h.parallel_for(sycl::nd_range<1>(gws_convert, lws_convert), [=](sycl::nd_item<1> item) {
        int64_t row = item.get_global_id(0);
        if (row < num_rows) {
          int64_t row_start = d_csr_offsets[row];
          int64_t row_end = d_csr_offsets[row + 1];

          for (int64_t idx = row_start; idx < row_end; idx++) {
            int64_t col = d_csr_columns[idx];
            float val = d_csr_values[idx];
            d_dense[row * num_cols + col] = val;
          }
        }
      });
    });
  }

  q.wait();
  auto end = std::chrono::steady_clock::now();
  auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  printf("Average execution time of SparseToDense_convert : %f (us)\\n", (time * 1e-3f) / repeat);

  // Copy results back
  q.memcpy(h_dense_result, d_dense, dense_size_bytes).wait();

  // Verify results
  int correct = 1;
  int64_t nnz = 0;
  for (int64_t i = 0; i < dense_size; i++) {
    if (h_dense_result[i] != 0) nnz++;

    if (h_dense[i] != h_dense_result[i]) {
      printf("@%ld: %f != %f\\n", i, h_dense[i], h_dense_result[i]);
      correct = 0;
      break;
    }
  }

  if (nnz != h_nnz) {
    printf("nnz: %ld != %ld\\n", nnz, h_nnz);
    correct = 0;
  }

  if (correct)
      printf("sparse2dense_csr_example test PASSED\\n");
  else
      printf("sparse2dense_csr_example test FAILED: wrong result\\n");

  // Cleanup
  sycl::free(d_csr_offsets, q);
  sycl::free(d_csr_columns, q);
  sycl::free(d_csr_values, q);
  sycl::free(d_dense, q);
  free(h_dense);
  free(h_dense_result);
  free(h_csr_values);
  free(h_csr_columns);
  free(h_csr_offsets);
  return EXIT_SUCCESS;
}
