#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <chrono>
#include <sycl/sycl.hpp>
#include "utils.h"

int main(int argc, char *argv[])
{
  int repeat = 1;

  if (argc != 8) {
    printf("Single-precision sparse matrix-matrix multiplication into sparse matrix,\\n");
    printf("where the sparse matrix is represented in CSR (Compressed Sparse Row) storage format\\n");
    printf("Usage %s <M> <K> <N> <A_nnz> <B_nnz> <repeat> <verify>\\n", argv[0]);
    printf("SPMM (A, B, C) where (A: M * K, B: K * N, C: M * N)\\n");
    return 1;
  }

  int m, k, n, a_nnz, b_nnz, verify;

  m = atoi(argv[1]);
  k = atoi(argv[2]);
  n = atoi(argv[3]);
  a_nnz = atoi(argv[4]);
  b_nnz = atoi(argv[5]);
  repeat = atoi(argv[6]);
  verify = atoi(argv[7]);

  // Host problem definition
  const int A_num_rows = m;
  const int A_num_cols = k;
  const int A_nnz      = a_nnz;
  const int B_num_rows = A_num_cols;
  const int B_num_cols = n;
  const int B_nnz      = b_nnz;
  const int C_num_rows = A_num_rows;
  const int C_num_cols = B_num_cols;
  const int lda        = A_num_cols;
  const int ldb        = B_num_cols;
  const int A_size     = lda * A_num_rows;
  const int B_size     = ldb * B_num_rows;
  const int C_size     = C_num_rows * C_num_cols;

  float *hA = (float*) malloc (A_size * sizeof(float));
  float *hB = (float*) malloc (B_size * sizeof(float));

  const size_t A_value_size_bytes  = A_nnz * sizeof(float);
  const size_t A_colidx_size_bytes = A_nnz * sizeof(int);
  const size_t A_rowidx_size_bytes = (A_num_rows + 1) * sizeof(int);

  const size_t B_value_size_bytes  = B_nnz * sizeof(float);
  const size_t B_colidx_size_bytes = B_nnz * sizeof(int);
  const size_t B_rowidx_size_bytes = (B_num_rows + 1) * sizeof(int);

  float *hA_values = (float*) malloc (A_value_size_bytes);
  int *hA_columns = (int*) malloc (A_colidx_size_bytes);
  int *hA_offsets = (int*) malloc (A_rowidx_size_bytes);

  float *hB_values = (float*) malloc (B_value_size_bytes);
  int *hB_columns = (int*) malloc (B_colidx_size_bytes);
  int *hB_offsets = (int*) malloc (B_rowidx_size_bytes);

  init_matrix(hA, A_num_rows, A_num_cols, A_nnz);
  init_csr(hA_offsets, hA_values, hA_columns, hA,
           A_num_rows, A_num_cols, A_nnz);

  init_matrix(hB, B_num_rows, B_num_cols, B_nnz);
  init_csr(hB_offsets, hB_values, hB_columns, hB,
           B_num_rows, B_num_cols, B_nnz);

#ifdef USE_GPU
  sycl::queue q(sycl::gpu_selector_v, sycl::property::queue::in_order());
#else
  sycl::queue q(sycl::cpu_selector_v, sycl::property::queue::in_order());
#endif

  // Device memory management
  int *dA_offsets = sycl::malloc_device<int>(A_num_rows + 1, q);
  int *dA_columns = sycl::malloc_device<int>(A_nnz, q);
  float *dA_values = sycl::malloc_device<float>(A_nnz, q);
  float *dA_dense = sycl::malloc_device<float>(A_size, q);

  int *dB_offsets = sycl::malloc_device<int>(B_num_rows + 1, q);
  int *dB_columns = sycl::malloc_device<int>(B_nnz, q);
  float *dB_values = sycl::malloc_device<float>(B_nnz, q);
  float *dB_dense = sycl::malloc_device<float>(B_size, q);

  float *dC_dense = sycl::malloc_device<float>(C_size, q);
  int *dC_nnz_per_row = sycl::malloc_device<int>(C_num_rows, q);
  int *dC_offsets = sycl::malloc_device<int>(C_num_rows + 1, q);

  // Copy A and B to device
  q.memcpy(dA_offsets, hA_offsets, A_rowidx_size_bytes);
  q.memcpy(dA_columns, hA_columns, A_colidx_size_bytes);
  q.memcpy(dA_values, hA_values, A_value_size_bytes);

  q.memcpy(dB_offsets, hB_offsets, B_rowidx_size_bytes);
  q.memcpy(dB_columns, hB_columns, B_colidx_size_bytes);
  q.memcpy(dB_values, hB_values, B_value_size_bytes);
  q.wait();

  sycl::range<1> gws_a ((A_size + 255) / 256 * 256);
  sycl::range<1> lws_a (256);

  sycl::range<1> gws_b ((B_size + 255) / 256 * 256);
  sycl::range<1> lws_b (256);

  sycl::range<1> gws_a_rows ((A_num_rows + 255) / 256 * 256);
  sycl::range<1> lws_a_rows (256);

  sycl::range<1> gws_b_rows ((B_num_rows + 255) / 256 * 256);
  sycl::range<1> lws_b_rows (256);

  sycl::range<1> gws_c ((C_size + 255) / 256 * 256);
  sycl::range<1> lws_c (256);

  sycl::range<1> gws_c_rows ((C_num_rows + 255) / 256 * 256);
  sycl::range<1> lws_c_rows (256);

  q.wait();
  auto start = std::chrono::steady_clock::now();

  for (int iter = 0; iter < repeat; iter++) {
    // Step 1: Convert sparse A to dense
    q.submit([&](sycl::handler &h) {
      h.parallel_for(sycl::nd_range<1>(gws_a, lws_a), [=](sycl::nd_item<1> item) {
        int idx = item.get_global_id(0);
        if (idx < A_size) {
          dA_dense[idx] = 0.0f;
        }
      });
    });

    q.submit([&](sycl::handler &h) {
      h.parallel_for(sycl::nd_range<1>(gws_a_rows, lws_a_rows), [=](sycl::nd_item<1> item) {
        int row = item.get_global_id(0);
        if (row < A_num_rows) {
          int row_start = dA_offsets[row];
          int row_end = dA_offsets[row + 1];
          for (int idx = row_start; idx < row_end; idx++) {
            int col = dA_columns[idx];
            float val = dA_values[idx];
            dA_dense[row * A_num_cols + col] = val;
          }
        }
      });
    });

    // Step 2: Convert sparse B to dense
    q.submit([&](sycl::handler &h) {
      h.parallel_for(sycl::nd_range<1>(gws_b, lws_b), [=](sycl::nd_item<1> item) {
        int idx = item.get_global_id(0);
        if (idx < B_size) {
          dB_dense[idx] = 0.0f;
        }
      });
    });

    q.submit([&](sycl::handler &h) {
      h.parallel_for(sycl::nd_range<1>(gws_b_rows, lws_b_rows), [=](sycl::nd_item<1> item) {
        int row = item.get_global_id(0);
        if (row < B_num_rows) {
          int row_start = dB_offsets[row];
          int row_end = dB_offsets[row + 1];
          for (int idx = row_start; idx < row_end; idx++) {
            int col = dB_columns[idx];
            float val = dB_values[idx];
            dB_dense[row * B_num_cols + col] = val;
          }
        }
      });
    });

    // Step 3: Dense matrix multiplication C = A * B
    q.submit([&](sycl::handler &h) {
      h.parallel_for(sycl::nd_range<1>(gws_c, lws_c), [=](sycl::nd_item<1> item) {
        int idx = item.get_global_id(0);
        if (idx < C_size) {
          int row = idx / C_num_cols;
          int col = idx % C_num_cols;
          float sum = 0.0f;
          for (int k_idx = 0; k_idx < A_num_cols; k_idx++) {
            sum += dA_dense[row * A_num_cols + k_idx] * dB_dense[k_idx * B_num_cols + col];
          }
          dC_dense[idx] = sum;
        }
      });
    });

    // Step 4: Count non-zeros per row in C
    q.submit([&](sycl::handler &h) {
      h.parallel_for(sycl::nd_range<1>(gws_c_rows, lws_c_rows), [=](sycl::nd_item<1> item) {
        int row = item.get_global_id(0);
        if (row < C_num_rows) {
          int count = 0;
          for (int col = 0; col < C_num_cols; col++) {
            if (dC_dense[row * C_num_cols + col] != 0.0f) {
              count++;
            }
          }
          dC_nnz_per_row[row] = count;
        }
        if (row == 0) {
          dC_offsets[C_num_rows] = 0;
        }
      });
    });

    // Step 5: Prefix sum on row offsets
    q.submit([&](sycl::handler &h) {
      h.single_task([=]() {
        int sum = 0;
        for (int i = 0; i < C_num_rows; i++) {
          dC_offsets[i] = sum;
          sum += dC_nnz_per_row[i];
        }
        dC_offsets[C_num_rows] = sum;
      });
    });
  }

  q.wait();
  auto end = std::chrono::steady_clock::now();
  auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  printf("Average execution time of SPMM compute: %f (us)\\n", (time * 1e-3f) / repeat);

  // Get C_nnz from device
  int *hC_offsets_temp = (int*) malloc ((C_num_rows + 1) * sizeof(int));
  q.memcpy(hC_offsets_temp, dC_offsets, (C_num_rows + 1) * sizeof(int)).wait();
  int C_nnz = hC_offsets_temp[C_num_rows];

  printf("C_nnz = %d\\n", C_nnz);

  if (C_nnz > 0) {
    // Allocate memory for C columns and values
    int *dC_columns = sycl::malloc_device<int>(C_nnz, q);
    float *dC_values = sycl::malloc_device<float>(C_nnz, q);

    // Step 6: Fill values and column indices for C
    q.submit([&](sycl::handler &h) {
      h.parallel_for(sycl::nd_range<1>(gws_c_rows, lws_c_rows), [=](sycl::nd_item<1> item) {
        int row = item.get_global_id(0);
        if (row < C_num_rows) {
          int offset = dC_offsets[row];
          int idx = offset;
          for (int col = 0; col < C_num_cols; col++) {
            float val = dC_dense[row * C_num_cols + col];
            if (val != 0.0f) {
              dC_values[idx] = val;
              dC_columns[idx] = col;
              idx++;
            }
          }
        }
      });
    }).wait();

    if (verify) {
      printf("Computing the reference SPMM results..\\n");
      float *hC_values_ref;
      int *hC_columns_ref;
      int *hC_offsets_ref;
      int C_nnz_ref = spmm(hA, hB,
                           hC_values_ref, hC_offsets_ref, hC_columns_ref,
                           A_num_cols, A_num_rows, B_num_cols);

      int *hC_offsets_result = (int*) malloc ((C_num_rows + 1) * sizeof(int));
      int *hC_columns_result = (int*) malloc (C_nnz * sizeof(int));
      float *hC_values_result = (float*) malloc (C_nnz * sizeof(float));

      q.memcpy(hC_offsets_result, dC_offsets, (C_num_rows + 1) * sizeof(int));
      q.memcpy(hC_columns_result, dC_columns, C_nnz * sizeof(int));
      q.memcpy(hC_values_result, dC_values, C_nnz * sizeof(float));
      q.wait();

      int correct = 1;
      if (C_nnz != C_nnz_ref) {
        printf("C_nnz mismatch: %d != %d\\n", C_nnz, C_nnz_ref);
        correct = 0;
      }

      if (correct) {
        for (int i = 0; i <= C_num_rows; i++) {
          if (hC_offsets_result[i] != hC_offsets_ref[i]) {
            printf("Row offset mismatch at %d: %d != %d\\n", i, hC_offsets_result[i], hC_offsets_ref[i]);
            correct = 0;
            break;
          }
        }
      }

      if (correct) {
        for (int i = 0; i < C_nnz; i++) {
          if (hC_columns_result[i] != hC_columns_ref[i]) {
            printf("Column mismatch at %d: %d != %d\\n", i, hC_columns_result[i], hC_columns_ref[i]);
            correct = 0;
            break;
          }
          if (fabsf(hC_values_result[i] - hC_values_ref[i]) > 1e-3f) {
            printf("Value mismatch at %d: %f != %f\\n", i, hC_values_result[i], hC_values_ref[i]);
            correct = 0;
            break;
          }
        }
      }

      if (correct)
        printf("spmm_csr_example test PASSED\\n");
      else
        printf("spmm_csr_example test FAILED: wrong result\\n");

      free(hC_values_ref);
      free(hC_columns_ref);
      free(hC_offsets_ref);
      free(hC_offsets_result);
      free(hC_columns_result);
      free(hC_values_result);
    }

    sycl::free(dC_columns, q);
    sycl::free(dC_values, q);
  }

  // Cleanup
  sycl::free(dA_offsets, q);
  sycl::free(dA_columns, q);
  sycl::free(dA_values, q);
  sycl::free(dA_dense, q);
  sycl::free(dB_offsets, q);
  sycl::free(dB_columns, q);
  sycl::free(dB_values, q);
  sycl::free(dB_dense, q);
  sycl::free(dC_dense, q);
  sycl::free(dC_nnz_per_row, q);
  sycl::free(dC_offsets, q);

  free(hA);
  free(hB);
  free(hA_values);
  free(hA_columns);
  free(hA_offsets);
  free(hB_values);
  free(hB_columns);
  free(hB_offsets);
  free(hC_offsets_temp);

  return EXIT_SUCCESS;
}
