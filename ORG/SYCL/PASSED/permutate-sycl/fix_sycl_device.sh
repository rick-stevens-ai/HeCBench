#!/bin/bash

# Create a temporary file with the updated device selection code
cat > new_device_code.cpp << 'EOL'
#include <iostream>
#include <sycl/sycl.hpp>

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
        
        // If no GPU found, try CPU
        if (!device_found) {
            for (const auto& platform : platforms) {
                auto devices = platform.get_devices();
                for (const auto& device : devices) {
                    if (device.is_cpu()) {
                        selected_device = device;
                        device_found = true;
                        std::cout << "Falling back to CPU device: " << device.get_info<sycl::info::device::name>() << std::endl;
                        break;
                    }
                }
                if (device_found) break;
            }
        }
        
        if (!device_found) {
            std::cerr << "No suitable SYCL device found" << std::endl;
            return false;
        }
        
        // Create queue with the selected device
        sycl::property_list properties{sycl::property::queue::in_order()};
        sycl::queue queue(selected_device, properties);
        std::cout << "Using device: " << queue.get_device().get_info<sycl::info::device::name>() << std::endl;
        
        // Rest of your existing implementation...
        
    } catch (const sycl::exception& e) {
        std::cerr << "SYCL error: " << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Standard error: " << e.what() << std::endl;
        return false;
    }
    
    return true;
}
EOL

# Replace the existing gpu_permutation_testing.cpp with our new version
mv new_device_code.cpp gpu_permutation_testing.cpp

# Rebuild
make clean && make GPU=yes -j$(nproc)
