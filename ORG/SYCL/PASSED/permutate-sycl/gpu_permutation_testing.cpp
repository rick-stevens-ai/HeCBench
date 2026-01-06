#include <iostream>
#include <sycl/sycl.hpp>
#include <chrono>
#include "kernel_functions.hpp"
#include "device_functions.hpp"

bool gpu_permutation_testing(float *gpu_runtime, uint32_t *counts, float *results,
                        float min_p, float max_p,
                        unsigned char *data, uint32_t data_size,
                        uint32_t num_iterations, uint32_t burn_in,
                        uint32_t batch_size, uint32_t num_batches) {
    try {
        // Get all available platforms
        auto platforms = sycl::platform::get_platforms();
        
        // First try to find a GPU device
        sycl::device selected_device;
        bool device_found = false;
        
        for (const auto& platform : platforms) {
            auto devices = platform.get_devices();
            for (const auto& device : devices) {
                if (device.is_gpu() && !device.has(sycl::aspect::fp64)) {
                    selected_device = device;
                    device_found = true;
                    std::cout << "Selected GPU device: " << device.get_info<sycl::info::device::name>() << std::endl;
                    break;
                }
            }
            if (device_found) break;
        }
        
        if (!device_found) {
            std::cerr << "No suitable GPU device found" << std::endl;
            return false;
        }
        
        // Create queue with profiling enabled
        sycl::property_list properties{sycl::property::queue::enable_profiling()};
        sycl::queue queue(selected_device, properties);
        std::cout << "Using device: " << queue.get_device().get_info<sycl::info::device::name>() << std::endl;

        // Start timing
        auto start_time = std::chrono::high_resolution_clock::now();

        // Calculate mean and median for statistical tests
        float mean = 0.0f;
        for (uint32_t i = 0; i < data_size; i++) {
            mean += data[i];
        }
        mean /= data_size;

        // Simple median calculation
        float median = data[data_size / 2];  // This is a simplification

        // Allocate device memory
        uint8_t* d_data = sycl::malloc_device<uint8_t>(data_size, queue);
        uint8_t* d_shuffled = sycl::malloc_device<uint8_t>(data_size * num_iterations, queue);
        uint32_t* d_counts = sycl::malloc_device<uint32_t>(54, queue);  // For 18 tests * 3 counters
        
        // Initialize counts to zero
        queue.memset(d_counts, 0, sizeof(uint32_t) * 54).wait();
        
        // Copy input data to device
        queue.memcpy(d_data, data, data_size).wait();

        std::vector<sycl::event> kernel_events;

        // First kernel: Shuffling
        const int shuffling_local_size = 256;
        auto shuffle_event = queue.parallel_for(
            sycl::nd_range<1>(
                sycl::range<1>(((num_iterations + shuffling_local_size - 1) / shuffling_local_size) * shuffling_local_size),
                sycl::range<1>(shuffling_local_size)
            ),
            [=](sycl::nd_item<1> item) {
                auto idx = item.get_global_id(0);
                if (idx < num_iterations) {
                    shuffling_kernel(d_shuffled, d_data, data_size, num_iterations, item);
                }
            });
        kernel_events.push_back(shuffle_event);
        shuffle_event.wait();

        // Second kernel: Statistical tests
        const uint32_t num_blocks = 4;  // Based on the kernel implementation
        const int stats_local_size = 256;
        auto stats_event = queue.parallel_for(
            sycl::nd_range<1>(
                sycl::range<1>(((num_blocks * batch_size + stats_local_size - 1) / stats_local_size) * stats_local_size),
                sycl::range<1>(stats_local_size)
            ),
            [=](sycl::nd_item<1> item) {
                auto idx = item.get_global_id(0);
                if (idx < num_blocks * batch_size) {
                    statistical_tests_kernel(d_counts, results, mean, median,
                                        d_shuffled, 8, data_size, num_iterations,
                                        num_blocks, item);
                }
            });
        kernel_events.push_back(stats_event);
        stats_event.wait();

        // Copy results back to host
        queue.memcpy(counts, d_counts, sizeof(uint32_t) * 54).wait();

        // Free device memory
        sycl::free(d_data, queue);
        sycl::free(d_shuffled, queue);
        sycl::free(d_counts, queue);

        // Calculate total kernel execution time
        float total_kernel_time = 0.0f;
        for (const auto& event : kernel_events) {
            auto start = event.get_profiling_info<sycl::info::event_profiling::command_start>();
            auto end = event.get_profiling_info<sycl::info::event_profiling::command_end>();
            total_kernel_time += (end - start) * 1e-9f; // Convert nanoseconds to seconds
        }

        // Calculate total execution time including data transfers
        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> total_time = end_time - start_time;

        *gpu_runtime = total_time.count();
        std::cout << "Kernel execution time: " << total_kernel_time << " seconds" << std::endl;
        std::cout << "Total GPU time (including transfers): " << *gpu_runtime << " seconds" << std::endl;
        
    } catch (const sycl::exception& e) {
        std::cerr << "SYCL error: " << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Standard error: " << e.what() << std::endl;
        return false;
    }
    
    return true;
}
