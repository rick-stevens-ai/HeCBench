sycl::device device = sycl::device(sycl::gpu_selector_v);
if (!device.has(sycl::aspect::fp64)) {
    // Device supports only fp32
    queue = sycl::queue(device);
}
