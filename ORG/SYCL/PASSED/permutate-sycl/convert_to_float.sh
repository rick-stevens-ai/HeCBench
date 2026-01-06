#!/bin/bash

# Convert double to float in header.h
sed -i 's/double results/float results/g' header.h
sed -i 's/double mean/float mean/g' header.h
sed -i 's/double median/float median/g' header.h

# Convert in device_functions.hpp
sed -i 's/double \*/float \*/g' device_functions.hpp
sed -i 's/const double/const float/g' device_functions.hpp
sed -i 's/double avg/float avg/g' device_functions.hpp
sed -i 's/double temp/float temp/g' device_functions.hpp

# Convert in gpu_permutation_testing.cpp
sed -i 's/double \*/float \*/g' gpu_permutation_testing.cpp
sed -i 's/const double/const float/g' gpu_permutation_testing.cpp
sed -i 's/double gpu_runtime/float gpu_runtime/g' gpu_permutation_testing.cpp

# Convert in kernel_functions.hpp
sed -i 's/double \*/float \*/g' kernel_functions.hpp
sed -i 's/const double/const float/g' kernel_functions.hpp
sed -i 's/double result/float result/g' kernel_functions.hpp

# Make script executable
chmod +x convert_to_float.sh
