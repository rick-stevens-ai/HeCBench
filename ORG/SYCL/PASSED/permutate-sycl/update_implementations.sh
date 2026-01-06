#!/bin/bash

# Update statistical_test.cpp
sed -i 's/double calculate_/float calculate_/g' statistical_test.cpp
sed -i 's/void calculate_statistics(double \*/void calculate_statistics(float \*/g' statistical_test.cpp
sed -i 's/double \*dmean/float \*dmean/g' statistical_test.cpp
sed -i 's/double \*dmedian/float \*dmedian/g' statistical_test.cpp
sed -i 's/double sum/float sum/g' statistical_test.cpp
sed -i 's/double values/float values/g' statistical_test.cpp
sed -i 's/double excur/float excur/g' statistical_test.cpp
sed -i 's/double result/float result/g' statistical_test.cpp

# Update gpu_permutation_testing.cpp
sed -i 's/bool gpu_permutation_testing(double \*/bool gpu_permutation_testing(float \*/g' gpu_permutation_testing.cpp
sed -i 's/double \*results/float \*results/g' gpu_permutation_testing.cpp
sed -i 's/double mean/float mean/g' gpu_permutation_testing.cpp
sed -i 's/double median/float median/g' gpu_permutation_testing.cpp

# Build again
make clean && make GPU=yes
