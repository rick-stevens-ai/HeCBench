#!/bin/bash

# Update variable declarations in permutation_testing.cpp
sed -i 's/double dmean/float dmean/g' permutation_testing.cpp
sed -i 's/double dmedian/float dmedian/g' permutation_testing.cpp
sed -i 's/double results\[19\]/float results\[19\]/g' permutation_testing.cpp
sed -i 's/double comp_result/float comp_result/g' permutation_testing.cpp

# Update declarations in statistical_test.cpp
sed -i 's/double \*results/float \*results/g' statistical_test.cpp
sed -i 's/double dmean/float dmean/g' statistical_test.cpp
sed -i 's/double dmedian/float dmedian/g' statistical_test.cpp
sed -i 's/double \*out/float \*out/g' statistical_test.cpp
sed -i 's/const double/const float/g' statistical_test.cpp

# Update declarations in utils.cpp
sed -i 's/double \*results/float \*results/g' utils.cpp

# Run the build
make clean && make GPU=yes
