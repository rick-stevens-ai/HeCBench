#!/bin/bash

# Update math functions to use their float versions in kernel_functions.hpp
sed -i 's/std::abs(/std::fabsf(/g' kernel_functions.hpp
sed -i 's/std::pow(/std::powf(/g' kernel_functions.hpp
sed -i 's/std::sqrt(/std::sqrtf(/g' kernel_functions.hpp
sed -i 's/std::log(/std::logf(/g' kernel_functions.hpp
sed -i 's/std::exp(/std::expf(/g' kernel_functions.hpp

# Update math functions in device_functions.hpp
sed -i 's/std::abs(/std::fabsf(/g' device_functions.hpp
sed -i 's/std::pow(/std::powf(/g' device_functions.hpp
sed -i 's/std::sqrt(/std::sqrtf(/g' device_functions.hpp
sed -i 's/std::log(/std::logf(/g' device_functions.hpp
sed -i 's/std::exp(/std::expf(/g' device_functions.hpp

# Update gpu_permutation_testing.cpp
sed -i 's/std::abs(/std::fabsf(/g' gpu_permutation_testing.cpp
sed -i 's/std::pow(/std::powf(/g' gpu_permutation_testing.cpp
sed -i 's/std::sqrt(/std::sqrtf(/g' gpu_permutation_testing.cpp
sed -i 's/std::log(/std::logf(/g' gpu_permutation_testing.cpp
sed -i 's/std::exp(/std::expf(/g' gpu_permutation_testing.cpp

# Also add explicit casts to float where needed
sed -i 's/static_cast<double>/static_cast<float>/g' kernel_functions.hpp
sed -i 's/static_cast<double>/static_cast<float>/g' device_functions.hpp
sed -i 's/static_cast<double>/static_cast<float>/g' gpu_permutation_testing.cpp

# Build and run
make clean && make GPU=yes
