#include <sycl/sycl.hpp>
int main() {
  sycl::queue q;
  int* d = sycl::malloc_device<int>(32, q);
  q.submit([&](sycl::handler& h) {
    h.parallel_for(sycl::nd_range<1>(32, 32), [=](sycl::nd_item<1> item) {
      auto sg = item.get_sub_group();
      int v = sg.shuffle(5, 0);
      d[item.get_global_id(0)] = v;
    });
  });
  return 0;
}
