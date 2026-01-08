#ifndef COMMON_
#define COMMON_

#include <sycl/sycl.hpp>

using namespace sycl;
constexpr access::mode sycl_read       = access::mode::read;
constexpr access::mode sycl_write      = access::mode::write;
constexpr access::mode sycl_read_write = access::mode::read_write;
constexpr access::mode sycl_discard_read_write = access::mode::discard_read_write;
constexpr access::mode sycl_discard_write = access::mode::discard_write;
constexpr access::mode sycl_atomic     = access::mode::atomic;

constexpr access::target sycl_cmem     = access::target::device;
constexpr access::target sycl_gmem     = access::target::device;

// Using local_accessor instead of deprecated local target
#define SYCL_LOCAL_MEM(type, name, size) \
    local_accessor<type> name{size, h}

#endif
