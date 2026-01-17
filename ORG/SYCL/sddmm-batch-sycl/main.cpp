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
    printf("Single-precision batched dense matrix - dense matrix multiplication into sparse matrix,\\n");
    printf("where the sparse matrix is represented in CSR (Compressed Sparse Row) storage format\\n");
    printf("Usage %s <number of batches> <M> <K> <N> <number of non-zero elements> <repeat> <verify>\\n", argv[0]);
    printf("SDDM (A, B, C) where (A: M * K, B: K * N, C: M * N)\\n");
    return 1;
  }

  int b, m, k, n, nnz, verify;

  b = atoi(argv[1]); // number of batches
  m = atoi(argv[2]);
  k = atoi(argv[3]);
  n = atoi(argv[4]);
  nnz = atoi(argv[5]);
  repeat = atoi(argv[6]);
  verify = atoi(argv[7]);

  const int A_num_rows   = m;
  const int A_num_cols   = k;
  const int B_num_rows   = A_num_cols;
  const int B_num_cols   = n;
  const int C_nnz     = nnz;
  const int lda       = A_num_cols;
  const int ldb       = B_num_cols;
  const int A_size    = lda * A_num_rows;
  const int B_size    = ldb * B_num_rows;
  const int C_size    = A_num_rows * B_num_cols;

  const size_t value_size_bytes  = b * C_nnz * sizeof(float);
  const size_t colidx_size_bytes = b * C_nnz * sizeof(int);
  const size_t rowidx_size_bytes = b * (A_num_rows + 1) * sizeof(int);

  float *hA = (float*) malloc (b * A_size * sizeof(float));
  float *hB = (float*) malloc (b * B_size * sizeof(float));

  // contain the result of A * B
  float *hC = (float*) malloc (b * C_size * sizeof(float));

  float *hC_values = (float*) malloc (value_size_bytes);
  int *hC_columns = (int*) malloc (colidx_size_bytes);
  int *hC_offsets = (int*) malloc (rowidx_size_bytes);
  float *hC_result  = (float*) malloc (value_size_bytes);

  for (int i = 0; i < b; i++) {
    init_matrix(hA + i * A_size, A_num_rows, A_num_cols, A_size);
    init_matrix(hB + i * B_size, B_num_rows, B_num_cols, B_size);
    init_matrix(hC + i * C_size, A_num_rows, B_num_cols, C_nnz);

    // initialize the CSR structures for the C matrix
    init_csr(hC_offsets + i * (A_num_rows+1),
             hC_values + i * C_nnz,
             hC_columns + i * C_nnz,
             hC + i * C_size,
             A_num_rows, B_num_cols, C_nnz);
  }

  if (verify) {
    printf("Computing the reference SDDMM results (batch size = %d)..\\n", b);
    for (int i = 0; i < b; i++) {
      sddmm (hA + i * A_size,
             hB + i * B_size,
             hC + i * C_size,
             hC_result + i * C_nnz,
             hC_offsets + i * (A_num_rows+1),
             hC_columns + i * C_nnz,
             A_num_cols, A_num_rows, B_num_cols);
    }
    printf("Done\\n");
  }

  float alpha = 1.0f;
  float beta  = 0.0f;

#ifdef USE_GPU
  sycl::queue q(sycl::gpu_selector_v, sycl::property::queue::in_order());
#else
  sycl::queue q(sycl::cpu_selector_v, sycl::property::queue::in_order());
#endif

  // Device memory management
  int *dC_columns = sycl::malloc_device<int>(b * C_nnz, q);
  int *dC_offsets = sycl::malloc_device<int>(b * (A_num_rows + 1), q);
  float *dC_values = sycl::malloc_device<float>(b * C_nnz, q);
  float *dC_dense = sycl::malloc_device<float>(b * C_size, q);
  float *dB = sycl::malloc_device<float>(b * B_size, q);
  float *dA = sycl::malloc_device<float>(b * A_size, q);

  q.memcpy(dA, hA, b * A_size * sizeof(float));
  q.memcpy(dB, hB, b * B_size * sizeof(float));
  q.memcpy(dC_offsets, hC_offsets, rowidx_size_bytes);
  q.memcpy(dC_columns, hC_columns, colidx_size_bytes);
  q.memcpy(dC_values, hC_values, value_size_bytes);
  q.wait();

  sycl::range<1> gws_matmul ((b * C_size + 255) / 256 * 256);
  sycl::range<1> lws_matmul (256);

  sycl::range<1> gws_sample ((b * C_nnz + 255) / 256 * 256);
  sycl::range<1> lws_sample (256);

  q.wait();
  auto start = std::chrono::steady_clock::now();

  for (int iter = 0; iter < repeat; iter++) {
    // Step 1: Dense matrix multiplication C = A * B for all batches
    q.submit([&](sycl::handler &h) {
      h.parallel_for(sycl::nd_range<1>(gws_matmul, lws_matmul), [=](sycl::nd_item<1> item) {
        int idx = item.get_global_id(0);
        if (idx < b * C_size) {
          int batch = idx / C_size;
          int elem = idx % C_size;
          int row = elem / B_num_cols;
          int col = elem % B_num_cols;

          float sum = 0.0f;
          for (int kk = 0; kk < A_num_cols; kk++) {
            sum += dA[batch * A_size + row * A_num_cols + kk] *
                   dB[batch * B_size + kk * B_num_cols + col];
          }
          dC_dense[idx] = alpha * sum + beta * dC_dense[idx];
        }
      });
    });

    // Step 2: Sample non-zero positions from dense C into sparse C_values
    q.submit([&](sycl::handler &h) {
      h.parallel_for(sycl::nd_range<1>(gws_sample, lws_sample), [=](sycl::nd_item<1> item) {
        int idx = item.get_global_id(0);
        if (idx < b * C_nnz) {
          int batch = idx / C_nnz;
          int nnz_idx = idx % C_nnz;

          // Find which row this non-zero belongs to
          int row = 0;
          int *offsets = dC_offsets + batch * (A_num_rows + 1);
          int *columns = dC_columns + batch * C_nnz;

          for (int r = 0; r < A_num_rows; r++) {
            if (nnz_idx >= offsets[r] && nnz_idx < offsets[r + 1]) {
              row = r;
              break;
            }
          }

          int col = columns[nnz_idx];
          dC_values[idx] = dC_dense[batch * C_size + row * B_num_cols + col];
        }
      });
    });
  }

  q.wait();
  auto end = std::chrono::steady_clock::now();
  auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  printf("Average execution time of SDDMM: %f (us)\\n", (time * 1e-3f) / repeat);

  if (verify) {
    // device result check
    q.memcpy(hC_values, dC_values, value_size_bytes).wait();
    int correct = 1;

    for (int i = 0; i < b; i++) {
      float *v =  hC_values + i * C_nnz;
      float *r =  hC_result + i * C_nnz;
      for (int j = 0; j < C_nnz; j++) {
        if (fabsf(v[j] - r[j]) > 1e-3f) {
          printf("@batch %d element %d: %f != %f\\n", i, j, v[j], r[j]);
          correct = 0;
          break;
        }
      }
      if (!correct) break;
    }

    if (correct)
      printf("sddmm_csr_example test PASSED\\n");
    else
      printf("sddmm_csr_example test FAILED: wrong result\\n");
  }

  // Cleanup
  sycl::free(dA, q);
  sycl::free(dB, q);
  sycl::free(dC_offsets, q);
  sycl::free(dC_columns, q);
  sycl::free(dC_values, q);
  sycl::free(dC_dense, q);

  free(hA);
  free(hB);
  free(hC);
  free(hC_values);
  free(hC_columns);
  free(hC_offsets);
  free(hC_result);

  return EXIT_SUCCESS;
}
