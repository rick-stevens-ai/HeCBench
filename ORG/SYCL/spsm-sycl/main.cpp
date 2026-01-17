#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <chrono>
#include <sycl/sycl.hpp>
#include "utils.h"

int main(int argc, char *argv[])
{
  int repeat = 1;

  if (argc != 6) {
    printf("The function solves a system of linear equations whose ");
    printf("coefficients are represented in a sparse triangular matrix.\\n");
    printf("The sparse matrix is represented in CSR (Compressed Sparse Row) storage format\\n");

    printf("Usage %s <M> <N> <A_nnz> <repeat> <verify>\\n", argv[0]);
    printf("SPSM (A, B, C) where (A: M * M, C: M * N, B: M * N)\\n");
    return 1;
  }

  int m, n, a_nnz, verify;

  m = atoi(argv[1]);
  n = atoi(argv[2]);
  a_nnz = atoi(argv[3]);
  repeat = atoi(argv[4]);
  verify = atoi(argv[5]);

  // Host problem definition
  const int A_num_rows = m;  // a square matrix
  const int A_num_cols = m;
  const int A_nnz      = a_nnz;
  const int lda        = A_num_cols;
  const int A_size     = lda * A_num_rows;

  const int C_num_rows = m;
  const int C_num_cols = n;
  const int ldc        = C_num_cols;
  const int C_size     = ldc * C_num_rows;

  const int B_num_rows = m;
  const int B_num_cols = n;
  const int ldb        = B_num_cols;
  const int B_size     = ldb * B_num_rows;

  const int nrhs = n;

  float *hA = (float*) malloc (A_size * sizeof(float));
  float *hB = (float*) malloc (B_size * sizeof(float));
  float *hC = (float*) malloc (C_size * sizeof(float));

  const size_t A_value_size_bytes  = A_nnz * sizeof(float);
  const size_t A_colidx_size_bytes = A_nnz * sizeof(int);
  const size_t A_rowidx_size_bytes = (A_num_rows + 1) * sizeof(int);

  float *hA_values = (float*) malloc (A_value_size_bytes);
  int *hA_columns = (int*) malloc (A_colidx_size_bytes);
  int *hA_offsets = (int*) malloc (A_rowidx_size_bytes);

  printf("Initializing host matrices..\\n");
  init_matrix(hA, A_num_rows, A_num_cols, A_nnz);
  init_csr(hA_offsets, hA_values, hA_columns, hA,
           A_num_rows, A_num_cols, A_nnz);

  init_matrix(hC, C_num_rows, C_num_cols, C_size);

  // precompute hB (B = A * C)
  spsm (hA, hC, hB, A_num_rows, C_num_cols);
  printf("Done\\n");

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
  float *dB = sycl::malloc_device<float>(B_size, q);
  float *dC = sycl::malloc_device<float>(C_size, q);

  q.memcpy(dA_offsets, hA_offsets, A_rowidx_size_bytes);
  q.memcpy(dA_columns, hA_columns, A_colidx_size_bytes);
  q.memcpy(dA_values, hA_values, A_value_size_bytes);
  q.memcpy(dB, hB, B_size * sizeof(float));
  q.memcpy(dC, hC, C_size * sizeof(float));
  q.wait();

  sycl::range<1> gws_a ((A_size + 255) / 256 * 256);
  sycl::range<1> lws_a (256);

  sycl::range<1> gws_a_rows ((A_num_rows + 255) / 256 * 256);
  sycl::range<1> lws_a_rows (256);

  sycl::range<1> gws_cols ((C_num_cols + 255) / 256 * 256);
  sycl::range<1> lws_cols (256);

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

    // Step 2: Forward substitution solve A * C = B for C
    // Solve each column independently (parallelize over columns)
    // Within each column, rows must be solved sequentially
    q.submit([&](sycl::handler &h) {
      h.parallel_for(sycl::nd_range<1>(gws_cols, lws_cols), [=](sycl::nd_item<1> item) {
        int col = item.get_global_id(0);
        if (col < C_num_cols) {
          // Forward substitution for column col
          for (int row = 0; row < A_num_rows; row++) {
            float sum = dB[row * B_num_cols + col];

            // Subtract contributions from previous rows
            for (int k = 0; k < row; k++) {
              sum -= dA_dense[row * A_num_cols + k] * dC[k * C_num_cols + col];
            }

            // Divide by diagonal element
            float diag = dA_dense[row * A_num_cols + row];
            if (diag != 0.0f) {
              dC[row * C_num_cols + col] = sum / diag;
            } else {
              dC[row * C_num_cols + col] = 0.0f;
            }
          }
        }
      });
    });
  }

  q.wait();
  auto end = std::chrono::steady_clock::now();
  auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  printf("Average execution time of SpSM solve: %f (us)\\n", (time * 1e-3f) / repeat);

  if (verify) {
    printf("Checking results..\\n");
    float *hY = (float*) malloc (C_size * sizeof(float));
    q.memcpy(hY, dC, nrhs * A_num_rows * sizeof(float)).wait();

    // compute hB2 = A * hY to verify
    float *hB2 = (float*) malloc (B_size * sizeof(float));
    spsm (hA, hY, hB2, A_num_rows, C_num_cols);

    int correct = 1;
    for (int i = 0; i < A_num_rows * C_num_cols; i++) {
      if (fabsf(hB[i] - hB2[i]) > 1e-2f) {
        printf("@%d %f != %f\\n", i, hB[i], hB2[i]);
        correct = 0;
        break;
      }
    }
    if (correct)
      printf("spsm_csr_example test PASSED\\n");
    else
      printf("spsm_csr_example test FAILED: wrong result\\n");

    free(hY);
    free(hB2);
  }

  // Cleanup
  sycl::free(dA_offsets, q);
  sycl::free(dA_columns, q);
  sycl::free(dA_values, q);
  sycl::free(dA_dense, q);
  sycl::free(dB, q);
  sycl::free(dC, q);

  free(hA);
  free(hB);
  free(hC);
  free(hA_values);
  free(hA_columns);
  free(hA_offsets);

  return EXIT_SUCCESS;
}
