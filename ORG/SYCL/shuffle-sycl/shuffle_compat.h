#ifndef SHUFFLE_COMPAT_H
#define SHUFFLE_COMPAT_H

#include <sycl/sycl.hpp>

// SYCL 2020 compatible shuffle operations using group algorithms

namespace shuffle_compat {

// Emulate shuffle using group_broadcast
template<typename T>
inline T shuffle(sycl::sub_group sg, T value, size_t src_lane) {
  return sycl::group_broadcast(sg, value, src_lane);
}

// Emulate shuffle_xor using shifts and broadcasts
// Note: This is an approximation that works for power-of-2 patterns
template<typename T>
inline T shuffle_xor(sycl::sub_group sg, T value, size_t mask) {
  auto local_id = sg.get_local_linear_id();
  auto src_lane = local_id ^ mask;
  return sycl::group_broadcast(sg, value, src_lane);
}

// Emulate shuffle_down using shift_group_right
template<typename T>
inline T shuffle_down(sycl::sub_group sg, T value, size_t delta) {
  return sycl::shift_group_right(sg, value, delta);
}

} // namespace shuffle_compat

#endif // SHUFFLE_COMPAT_H
