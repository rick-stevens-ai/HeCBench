#include <stdio.h>
#include <stdlib.h>
#include <sycl/sycl.hpp>
#include "xpu_stubs.h"

// Function declarations from timex.cpp
void gpu_forward(const float *w, const float *k, float *x, float eps, int B, int C, int T);
void gpu_backward(const float *w, const float *k, const float *gwk, float *gw, float *gk, int B, int C, int T);

int main(int argc, char* argv[]) {
    // Initialize SYCL queue
    init_global_queue();

    // Test parameters
    int B = 4;   // Batch size
    int C = 8;   // Channels
    int T = 256; // Time dimension
    float eps = 1e-6f;

    printf("dwconv1d-sycl benchmark\n");
    printf("B=%d, C=%d, T=%d\n", B, C, T);

    // Allocate host memory
    size_t w_size = C * T * sizeof(float);
    size_t k_size = B * C * T * sizeof(float);
    size_t x_size = B * C * T * sizeof(float);

    float *h_w = (float*)malloc(w_size);
    float *h_k = (float*)malloc(k_size);
    float *h_x = (float*)malloc(x_size);
    float *h_gwk = (float*)malloc(x_size);
    float *h_gw = (float*)malloc(w_size);
    float *h_gk = (float*)malloc(k_size);

    // Initialize input data
    for (int i = 0; i < C * T; i++) {
        h_w[i] = (float)rand() / RAND_MAX;
    }
    for (int i = 0; i < B * C * T; i++) {
        h_k[i] = (float)rand() / RAND_MAX;
        h_gwk[i] = (float)rand() / RAND_MAX;
    }

    // Allocate device memory
    float *d_w = sycl::malloc_device<float>(C * T, get_global_queue());
    float *d_k = sycl::malloc_device<float>(B * C * T, get_global_queue());
    float *d_x = sycl::malloc_device<float>(B * C * T, get_global_queue());
    float *d_gwk = sycl::malloc_device<float>(B * C * T, get_global_queue());
    float *d_gw = sycl::malloc_device<float>(C * T, get_global_queue());
    float *d_gk = sycl::malloc_device<float>(B * C * T, get_global_queue());

    // Copy data to device
    get_global_queue().memcpy(d_w, h_w, w_size).wait();
    get_global_queue().memcpy(d_k, h_k, k_size).wait();
    get_global_queue().memcpy(d_gwk, h_gwk, x_size).wait();

    printf("Running forward pass...\n");
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100; i++) {
        gpu_forward(d_w, d_k, d_x, eps, B, C, T);
    }
    get_global_queue().wait();
    auto end = std::chrono::high_resolution_clock::now();
    double forward_time = std::chrono::duration<double, std::milli>(end - start).count();
    printf("Forward pass: %.2f ms (average over 100 iterations)\n", forward_time / 100.0);

    printf("Running backward pass...\n");
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100; i++) {
        gpu_backward(d_w, d_k, d_gwk, d_gw, d_gk, B, C, T);
    }
    get_global_queue().wait();
    end = std::chrono::high_resolution_clock::now();
    double backward_time = std::chrono::duration<double, std::milli>(end - start).count();
    printf("Backward pass: %.2f ms (average over 100 iterations)\n", backward_time / 100.0);

    // Copy results back
    get_global_queue().memcpy(h_x, d_x, x_size).wait();
    get_global_queue().memcpy(h_gw, d_gw, w_size).wait();
    get_global_queue().memcpy(h_gk, d_gk, k_size).wait();

    // Verify results (simple checksum)
    double sum = 0.0;
    for (int i = 0; i < B * C * T; i++) {
        sum += h_x[i];
    }
    printf("Forward result checksum: %.6f\n", sum);

    // Cleanup
    sycl::free(d_w, get_global_queue());
    sycl::free(d_k, get_global_queue());
    sycl::free(d_x, get_global_queue());
    sycl::free(d_gwk, get_global_queue());
    sycl::free(d_gw, get_global_queue());
    sycl::free(d_gk, get_global_queue());

    free(h_w);
    free(h_k);
    free(h_x);
    free(h_gwk);
    free(h_gw);
    free(h_gk);

    printf("Test completed successfully!\n");
    return 0;
}
