#!/bin/bash

# Create a temporary file with the updated device selection code
cat > gpu_device_patch.cpp << 'EOL'
try {
    // Try GPU first
    sycl::device dev = sycl::device(sycl::gpu_selector_v);
    if (dev.has(sycl::aspect::fp16) || dev.has(sycl::aspect::fp32)) {
        queue = sycl::queue(dev, {sycl::property::queue::enable_profiling()});
        std::cout << "Using GPU device: " << dev.get_info<sycl::info::device::name>() << std::endl;
    }
} catch (const sycl::exception& e) {
    std::cerr << "Failed to select GPU device: " << e.what() << std::endl;
    std::cerr << "Falling back to CPU" << std::endl;
    
    // Fall back to CPU
    try {
        sycl::device dev = sycl::device(sycl::cpu_selector_v);
        queue = sycl::queue(dev, {sycl::property::queue::enable_profiling()});
        std::cout << "Using CPU device: " << dev.get_info<sycl::info::device::name>() << std::endl;
    } catch (const sycl::exception& e) {
        std::cerr << "Failed to select CPU device: " << e.what() << std::endl;
        throw;
    }
}
EOL

# Patch the file
sed -i '/sycl::queue queue/r gpu_device_patch.cpp' gpu_permutation_testing.cpp

# Clean up and rebuild
rm -f gpu_device_patch.cpp
make clean && make GPU=yes
