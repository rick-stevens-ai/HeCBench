#!/bin/bash

# Update double casts to float in device_functions.hpp
sed -i 's/*out_average = avg \/ (double)cnt/*out_average = avg \/ (float)cnt/g' device_functions.hpp
sed -i 's/*out_max = (double)max/*out_max = (float)max/g' device_functions.hpp
sed -i 's/*out_num = (double)num_runs/*out_num = (float)num_runs/g' device_functions.hpp
sed -i 's/*out_len = (double)max_len_runs/*out_len = (float)max_len_runs/g' device_functions.hpp
sed -i 's/*out_max = (double)sycl::max/*out_max = (float)sycl::max/g' device_functions.hpp

# Add SYCL aspect requirements for device selector to enable float-only mode
cat > selector_patch.cpp << 'EOL'
sycl::device device = sycl::device(sycl::gpu_selector_v);
if (!device.has(sycl::aspect::fp64)) {
    // Device supports only fp32
    queue = sycl::queue(device);
}
EOL

# Build again
make clean && make GPU=yes
