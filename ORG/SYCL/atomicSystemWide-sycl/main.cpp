#include <sycl/sycl.hpp>
#include <iostream>
#include <vector>
#include <string>

using namespace sycl;

// Helper function to check device capabilities
bool verify_device_capabilities(const device& dev) {
    std::cout << "Checking device capabilities...\n";
    
    auto atomic_caps = dev.get_info<info::device::atomic_memory_scope_capabilities>();
    bool has_system_scope = false;
    
    std::cout << "Supported atomic memory scopes:\n";
    for (const auto& scope : atomic_caps) {
        std::cout << "  - Scope " << static_cast<int>(scope);
        switch(scope) {
            case memory_scope::work_item: std::cout << " (work_item)\n"; break;
            case memory_scope::work_group: std::cout << " (work_group)\n"; break;
            case memory_scope::device: std::cout << " (device)\n"; break;
            case memory_scope::system: 
                std::cout << " (system)\n";
                has_system_scope = true;
                break;
            default: std::cout << " (unknown)\n";
        }
    }
    
    if (!has_system_scope) {
        std::cout << "WARNING: System-wide atomic operations not supported!\n";
        return false;
    }
    
    return true;
}

class AtomicKernel {
private:
    int loop_count;

public:
    AtomicKernel(int count) : loop_count(count) {}

    void operator()(handler& cgh, buffer<int>& counterBuf, buffer<int>& errorBuf) {
        auto counter = counterBuf.get_access<access::mode::atomic>(cgh);
        auto errors = errorBuf.get_access<access::mode::write>(cgh);
        int local_loop_count = loop_count;  // Create local copy for kernel

        cgh.parallel_for(range<1>(1024), [=](id<1> idx) {
            atomic<int, access::address_space::global_space> atom(counter[0]);
            for(int i = 0; i < local_loop_count; i++) {
                auto old_val = atom.fetch_add(1, memory_order::relaxed);
                if (old_val >= 1024 * local_loop_count) {
                    errors[idx[0]] = 1;
                }
            }
        });
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <loop count within the kernel>\n";
        return 1;
    }

    int loop_count = std::stoi(argv[1]);
    constexpr int ITERATIONS = 10;
    std::vector<int> results(ITERATIONS);

    try {
        device dev = device(default_selector_v);
        
        std::cout << "Selected device: " << dev.get_info<info::device::name>() << "\n\n";
        
        if (!verify_device_capabilities(dev)) {
            std::cerr << "Device capabilities check failed!\n";
            return 1;
        }

        queue q(dev, [](exception_list el) {
            for (auto& e : el) {
                try {
                    std::rethrow_exception(e);
                } catch (const exception& e) {
                    std::cerr << "Caught asynchronous SYCL exception: " << e.what() << std::endl;
                }
            }
        });

        AtomicKernel kernel(loop_count);

        for (int i = 0; i < ITERATIONS; ++i) {
            std::vector<int> counter(1, 0);
            std::vector<int> errors(1024, 0);
            {
                buffer<int> counterBuf(counter.data(), range<1>(1));
                buffer<int> errorBuf(errors.data(), range<1>(1024));
                q.submit([&](handler& cgh) {
                    kernel(cgh, counterBuf, errorBuf);
                });
            }
            results[i] = counter[0];
            
            // Check for errors
            int error_count = 0;
            for (int err : errors) {
                error_count += err;
            }
            
            std::cout << "Iteration " << i << ": Final counter = " << counter[0]
                    << ", Errors = " << error_count << "\n";
        }

        // Verify results
        bool all_correct = true;
        int expected_value = 1024 * loop_count;
        for (int i = 0; i < ITERATIONS; ++i) {
            if (results[i] != expected_value) {
                std::cout << "Iteration " << i << " failed: Expected " << expected_value 
                        << ", got " << results[i] << "\n";
                all_correct = false;
            }
        }

        if (all_correct) {
            std::cout << "\nAll iterations completed successfully!\n";
        } else {
            std::cout << "\nSome iterations failed!\n";
            return 1;
        }

    } catch (const exception& e) {
        std::cerr << "SYCL exception caught: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
