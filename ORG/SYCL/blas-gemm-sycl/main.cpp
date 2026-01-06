#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sycl/sycl.hpp>
#include "oneapi/mkl/blas.hpp"
#include "mkl.h"
#include "utils.h"

#define TILE_X 16
#define TILE_Y 16

template <typename T>
void matrix_mul(sycl::nd_item<2> &item, T *a, T *b, T *c, int M, int K, int N, T alpha, T beta) {
  int row = item.get_global_id(0);
  int col = item.get_global_id(1);
  if (row < M && col < N) {
    T s = 0;
    for (int k = 0; k < K; k++)
      s += a[row * K + k] * b[k * N + col];
    c[row * N + col] = alpha * s + beta * c[row * N + col];
  }
}

template <typename T>
void run_simple_gemm(sycl::queue &q, T *a, T *b, T *c, int M, int K, int N, T alpha, T beta) {
  sycl::range<2> gws ((M + TILE_Y - 1) / TILE_Y * TILE_Y,
                      (N + TILE_X - 1) / TILE_X * TILE_X);
  sycl::range<2> lws (TILE_Y, TILE_X);

  q.submit([&] (sycl::handler &cgh) {
    cgh.parallel_for(sycl::nd_range<2>(gws, lws), [=] (sycl::nd_item<2> item) {
      matrix_mul(item, a, b, c, M, K, N, alpha, beta);
    });
  });
}

void run_gemm_example(MKL_INT m, MKL_INT k, MKL_INT n, int repeat) {
  oneapi::mkl::transpose transA = oneapi::mkl::transpose::nontrans;
  oneapi::mkl::transpose transB = oneapi::mkl::transpose::nontrans;

  float alpha = 2.0f;
  float beta  = 0.5f;

  const size_t A_size = sizeof(float) * m * k;
  const size_t B_size = sizeof(float) * k * n;
  const size_t C_size = sizeof(float) * m * n;

  float* a = (float *)mkl_malloc(A_size, 64);
  float* b = (float *)mkl_malloc(B_size, 64);
  float* c = (float *)mkl_malloc(C_size, 64);
  float* r = (float *)mkl_malloc(C_size, 64);

  srand(2);
  rand_matrix(a, m, k);
  rand_matrix(b, k, n);
  rand_matrix(c, m, n);

#ifdef USE_GPU
  sycl::queue q(sycl::gpu_selector_v, sycl::property::queue::in_order());
#else
  sycl::queue q(sycl::cpu_selector_v, sycl::property::queue::in_order());
#endif

  float *da, *db, *dc, *dr;
  da = sycl::malloc_device<float>(m*k, q);
  db = sycl::malloc_device<float>(k*n, q);
  dc = sycl::malloc_device<float>(m*n, q);
  dr = sycl::malloc_device<float>(m*n, q);
  q.memcpy(da, a, A_size);
  q.memcpy(db, b, B_size);
  q.memcpy(dc, c, B_size);
  q.memcpy(dr, c, B_size);

  std::cout << "Checking BLAS GEMM.. ";
  run_simple_gemm(q, da, db, dr, m, k, n, alpha, beta);

  oneapi::mkl::blas::gemm(q, transA, transB,
                          n, m, k, alpha, db, n, da, k, beta, dc, n);
  q.memcpy(c, dc, C_size).wait();
  q.memcpy(r, dr, C_size).wait();
  int error = memcmp(c, r, C_size);
  std::cout << (error ? "FAIL" : "PASS") << std::endl;

  q.wait();
  auto start = std::chrono::steady_clock::now();

  for (int i = 0; i < repeat; i++) {
    oneapi::mkl::blas::gemm(q, transA, transB, n, m, k,
                            alpha, db, n, da, k, beta, dc, n);
  }

  q.wait();
  auto end = std::chrono::steady_clock::now();
  auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  performance(m, n, k, false, time / repeat);

#ifdef DEBUG
  std::cout << "\n\t\tOutputting 2x2 block of A,B,C matrices:" << std::endl;
  print_2x2_matrix_values(a, k, "A");
  print_2x2_matrix_values(b, n, "B");
  q.memcpy(c, dc, C_size).wait();
  print_2x2_matrix_values(c, n, "C");
#endif

  sycl::free(da, q);
  sycl::free(db, q);
  sycl::free(dc, q);
  sycl::free(dr, q);

  mkl_free(a);
  mkl_free(b);
  mkl_free(c);
  mkl_free(r);
}

int main (int argc, char ** argv) {
  if (argc != 5) {
    printf("Usage: %s <m> <k> <n> <repeat>\n", argv[0]);
    return 1;
  }
  const int m = atoi(argv[1]);
  const int k = atoi(argv[2]);
  const int n = atoi(argv[3]);
  const int repeat = atoi(argv[4]);

  std::cout << "\tRunning with single precision data type:" << std::endl;
  run_gemm_example(m, k, n, repeat);

  return 0;
}
