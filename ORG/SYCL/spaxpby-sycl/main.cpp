#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <chrono>
#include <sycl/sycl.hpp>
#include "utils.h"

int main(int argc, char *argv[])
{
  int repeat = 1;

  if (argc != 5) {
    printf("The function computes the sum of a sparse vector and a dense vector ");
    printf("in single-precision floating-point operations\n");
    printf("for i=0 to n-1        \n");
    printf("    Y[i] = beta * Y[i]\n");
    printf("for i=0 to nnz-1      \n");
    printf("    Y[X_indices[i]] += alpha * X_values[i]\n");
    printf("\n");
    printf("Usage %s <M> <N> <nnz> <repeat>\n", argv[0]);
    printf("The size of the vector (n) is M * N\n");
    printf("nnz is the number of non-zero elements\n");
    return 1;
  }

  size_t m, n, nnz;

  m = atol(argv[1]);
  n = atol(argv[2]);
  nnz = atol(argv[3]);
  repeat = atoi(argv[4]);

  // Host problem definition
  const size_t size = m * n;

  const size_t value_size_bytes = nnz * sizeof(float);
  const size_t index_size_bytes = nnz * sizeof(size_t);
  const size_t size_bytes = size * sizeof(float);

  float *hA = (float*) malloc (size_bytes);
  float *hB = (float*) malloc (size_bytes); // the initial and final results computed on a host
  float *hY = (float*) malloc (size_bytes); // the results from a device
  float *hA_values = (float*) malloc (value_size_bytes);
  size_t *hA_indices = (size_t*) malloc (index_size_bytes);

  printf("Initializing input matrices..\n");
  init_matrix(hA, m, n, nnz);

  size_t k = 0;
  for (size_t i = 0; i < size; i++) {
    if (hA[i] != 0) {
      hA_indices[k] = i;
      hA_values[k] = hA[i];
      k++;
    }
  }

  init_matrix(hB, m, n, size);

  printf("Done\n");

  const float alpha = 1.0f;
  const float beta  = 1.0f;

#ifdef USE_GPU
  sycl::queue q(sycl::gpu_selector_v, sycl::property::queue::in_order());
#else
  sycl::queue q(sycl::cpu_selector_v, sycl::property::queue::in_order());
#endif

  size_t *dX_indices = sycl::malloc_device<size_t>(nnz, q);
  float *dX_values = sycl::malloc_device<float>(nnz, q);
  float *dY = sycl::malloc_device<float>(size, q);

  q.memcpy(dX_indices, hA_indices, index_size_bytes);
  q.memcpy(dX_values, hA_values, value_size_bytes);
  q.memcpy(dY, hB, size_bytes);
  q.wait();

  sycl::range<1> gws_scale ((size + 255) / 256 * 256);
  sycl::range<1> lws_scale (256);

  sycl::range<1> gws_axpy ((nnz + 255) / 256 * 256);
  sycl::range<1> lws_axpy (256);

  q.wait();
  auto start = std::chrono::steady_clock::now();

  for (int i = 0; i < repeat; i++) {
    // Y = beta * Y
    q.submit([&](sycl::handler &h) {
      h.parallel_for(sycl::nd_range<1>(gws_scale, lws_scale), [=](sycl::nd_item<1> item) {
        size_t idx = item.get_global_id(0);
        if (idx < size) {
          dY[idx] = beta * dY[idx];
        }
      });
    });

    // Y[X_indices[i]] += alpha * X_values[i]
    q.submit([&](sycl::handler &h) {
      h.parallel_for(sycl::nd_range<1>(gws_axpy, lws_axpy), [=](sycl::nd_item<1> item) {
        size_t idx = item.get_global_id(0);
        if (idx < nnz) {
          size_t y_idx = dX_indices[idx];
          dY[y_idx] += alpha * dX_values[idx];
        }
      });
    });
  }

  q.wait();
  auto end = std::chrono::steady_clock::now();
  auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  printf("Average execution time of SPAXPBY : %f (us)\n", (time * 1e-3f) / repeat);

  // device result check
  q.memcpy(hY, dY, size_bytes).wait();

  printf("Computing the reference results..\n");
  for (int n = 0; n < repeat; n++) {
    for (size_t i = 0; i < size; i++) {
      hB[i] = alpha * hA[i] + beta * hB[i];
    }
  }
  printf("Done\n");

  int correct = 1;
  for (size_t i = 0; i < size; i++) {
    if (fabsf(hY[i] - hB[i]) > 1e-2f) {
      correct = 0;
      break;
    }
  }
  if (correct)
      printf("axpby_example test PASSED\n");
  else
      printf("axpby_example test FAILED: wrong result\n");

  // device memory deallocation
  sycl::free(dX_indices, q);
  sycl::free(dX_values, q);
  sycl::free(dY, q);
  free(hA);
  free(hB);
  free(hY);
  free(hA_values);
  free(hA_indices);
  return EXIT_SUCCESS;
}
