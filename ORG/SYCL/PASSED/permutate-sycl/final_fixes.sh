#!/bin/bash

# Update any remaining double precision operations in kernel_functions.hpp
sed -i 's/double(/float(/g' kernel_functions.hpp
sed -i 's/double sum/float sum/g' kernel_functions.hpp
sed -i 's/double tmp/float tmp/g' kernel_functions.hpp
sed -i 's/double value/float value/g' kernel_functions.hpp
sed -i 's/\.0/\.0f/g' kernel_functions.hpp
sed -i 's/double /float /g' kernel_functions.hpp

# Make sure all numeric literals are float in gpu_permutation_testing.cpp
sed -i 's/\.0/\.0f/g' gpu_permutation_testing.cpp

# Update any remaining doubles in device_functions.hpp
sed -i 's/double /float /g' device_functions.hpp
sed -i 's/\.0/\.0f/g' device_functions.hpp

# Build and run
make clean && make GPU=yes
