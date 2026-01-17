#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <sycl/sycl.hpp>
#include "utils.h"

int main(int argc, char *argv[])
{
  int repeat = 1;

  if (argc != 5) {
    printf("This function computes the number of nonzero elements per row or column");
    printf(" and the total number of nonzero elements in a dense matrix.\n");
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
  const int64_t dense_size_bytes = dense_size * sizeof(float);
  const int64_t nnzRowCol_size_bytes = num_rows * sizeof(int);

  float *h_dense = (float*) malloc (dense_size_bytes);

  printf("Initializing host matrices..\n");
  init_matrix(h_dense, num_rows, num_cols, h_nnz);

  // device results
  int* nnzPerRowColumn = (int*) malloc (nnzRowCol_size_bytes);

#ifdef USE_GPU
  sycl::queue q(sycl::gpu_selector_v, sycl::property::queue::in_order());
#else
  sycl::queue q(sycl::cpu_selector_v, sycl::property::queue::in_order());
#endif

  // allocate device memory
  float *d_dense = sycl::malloc_device<float>(dense_size, q);
  int *d_nnzPerRowColumn = sycl::malloc_device<int>(num_rows, q);
  int *d_nnzTotal = sycl::malloc_device<int>(1, q);

  q.memcpy(d_dense, h_dense, dense_size_bytes);
  q.wait();

  // SYCL kernel to count non-zeros per row
  sycl::range<1> gws ((num_rows + 255) / 256 * 256);
  sycl::range<1> lws (256);

  q.wait();
  auto start = std::chrono::steady_clock::now();

  for (int i = 0; i < repeat; i++) {
    // Initialize nnzTotal to 0
    q.memset(d_nnzTotal, 0, sizeof(int));

    // Count non-zeros per row
    q.submit([&](sycl::handler &h) {
      h.parallel_for(sycl::nd_range<1>(gws, lws), [=](sycl::nd_item<1> item) {
        int64_t row = item.get_global_id(0);
        if (row < num_rows) {
          int count = 0;
          for (int64_t col = 0; col < num_cols; col++) {
            if (d_dense[row * num_cols + col] != 0.0f) {
              count++;
            }
          }
          d_nnzPerRowColumn[row] = count;

          // Atomically add to total
          auto atomic_ref = sycl::atomic_ref<int,
            sycl::memory_order::relaxed,
            sycl::memory_scope::device,
            sycl::access::address_space::global_space>(d_nnzTotal[0]);
          atomic_ref.fetch_add(count);
        }
      });
    });
  }

  q.wait();
  auto end = std::chrono::steady_clock::now();
  auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
  printf("Average execution time of spnnz : %f (us)\n", (time * 1e-3f) / repeat);

  // Copy results back
  int nnzTotal = 0;
  q.memcpy(nnzPerRowColumn, d_nnzPerRowColumn, nnzRowCol_size_bytes);
  q.memcpy(&nnzTotal, d_nnzTotal, sizeof(int));
  q.wait();

  // Verify results
  int correct = 1;
  if (h_nnz != nnzTotal) {
    printf("nnz: %ld != %d\n", h_nnz, nnzTotal);
    correct = 0;
    goto print_error;
  }

  for (int64_t i = 0; i < num_rows; i++) {
    int nnz = 0; // nnz per row
    for (int64_t j = 0; j < num_cols; j++) {
      if (h_dense[i*num_cols+j] != 0) nnz++;
    }
    if (nnz != nnzPerRowColumn[i]) {
      printf("@row %ld %d != %d\n", i, nnz, nnzPerRowColumn[i]);
      correct = 0;
      goto print_error;
    }
  }

  print_error:
  if (correct)
      printf("sparse_nnz_example test PASSED\n");
  else
      printf("sparse_nnz_example test FAILED: wrong result\n");

  // Cleanup
  sycl::free(d_nnzPerRowColumn, q);
  sycl::free(d_dense, q);
  sycl::free(d_nnzTotal, q);
  free(h_dense);
  free(nnzPerRowColumn);

  return correct ? EXIT_SUCCESS : EXIT_FAILURE;
}
