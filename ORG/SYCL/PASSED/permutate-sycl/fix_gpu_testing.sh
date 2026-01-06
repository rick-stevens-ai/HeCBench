#!/bin/bash

# Update the gpu_permutation_testing signature in header.h to use float consistently
sed -i 's/bool gpu_permutation_testing(float \*gpu_runtime, uint32_t \*counts, double \*results/bool gpu_permutation_testing(float \*gpu_runtime, uint32_t \*counts, float \*results/g' header.h

# Update gpu_permutation_testing.cpp
sed -i 's/double \*results/float \*results/g' gpu_permutation_testing.cpp

# Build again
make clean && make GPU=yes
