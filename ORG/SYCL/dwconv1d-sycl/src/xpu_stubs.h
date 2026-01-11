#ifndef XPU_STUBS_H
#define XPU_STUBS_H

#include <sycl/sycl.hpp>

// Stub implementations for Intel XPU headers that aren't publicly available

// Global queue for SYCL operations
static sycl::queue* g_queue = nullptr;

inline void init_global_queue() {
    if (!g_queue) {
        g_queue = new sycl::queue(sycl::gpu_selector_v);
    }
}

inline sycl::queue& get_global_queue() {
    if (!g_queue) {
        init_global_queue();
    }
    return *g_queue;
}

#endif // XPU_STUBS_H
