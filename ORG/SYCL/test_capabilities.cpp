#include <sycl/sycl.hpp>
#include <iostream>

int main() {
    try {
        // Get default device
        sycl::device dev = sycl::device(sycl::default_selector_v);
        
        std::cout << "Device Info:\n";
        std::cout << "  Name: " << dev.get_info<sycl::info::device::name>() << "\n";
        std::cout << "  Vendor: " << dev.get_info<sycl::info::device::vendor>() << "\n";
        std::cout << "  Version: " << dev.get_info<sycl::info::device::version>() << "\n";
        
        // Memory capabilities
        std::cout << "\nMemory Capabilities:\n";
        std::cout << "  Global Memory Size: " << dev.get_info<sycl::info::device::global_mem_size>() << " bytes\n";
        std::cout << "  Local Memory Size: " << dev.get_info<sycl::info::device::local_mem_size>() << " bytes\n";
        
        // Atomic capabilities
        auto atomic_caps = dev.get_info<sycl::info::device::atomic_memory_scope_capabilities>();
        std::cout << "\nAtomic Memory Scope Support:\n";
        for (const auto& scope : atomic_caps) {
            std::cout << "  - " << static_cast<int>(scope) << "\n";
        }
        
        // Work-group capabilities
        std::cout << "\nWork-group Capabilities:\n";
        std::cout << "  Max Work-group Size: " << dev.get_info<sycl::info::device::max_work_group_size>() << "\n";
        
        return 0;
    } catch (const sycl::exception& e) {
        std::cerr << "SYCL exception caught: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Standard exception caught: " << e.what() << std::endl;
        return 2;
    }
}
