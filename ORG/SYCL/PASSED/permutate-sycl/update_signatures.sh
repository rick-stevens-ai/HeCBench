#!/bin/bash

# Update function signatures in header.h
sed -i 's/void print_original_test_statistics(double \*results)/void print_original_test_statistics(float \*results)/g' header.h
sed -i 's/void calculate_statistics(double \*dmean, double \*dmedian/void calculate_statistics(float \*dmean, float \*dmedian/g' header.h
sed -i 's/int run_tests(double \*results, double dmean, double dmedian/int run_tests(float \*results, float dmean, float dmedian/g' header.h
sed -i 's/void excursion_test(double \*out, const double dmean/void excursion_test(float \*out, const float dmean/g' header.h
sed -i 's/void directional_runs_and_number_of_inc_dec(double \*out_num, double \*out_len, double \*out_max/void directional_runs_and_number_of_inc_dec(float \*out_num, float \*out_len, float \*out_max/g' header.h
sed -i 's/void runs_based_on_median(double \*out_num, double \*out_len, const double dmedian/void runs_based_on_median(float \*out_num, float \*out_len, const float dmedian/g' header.h
sed -i 's/int collision_test_statistic(double \*out_avg, double \*out_max/int collision_test_statistic(float \*out_avg, float \*out_max/g' header.h
sed -i 's/void periodicity_covariance_test(double \*out_num, double \*out_strength/void periodicity_covariance_test(float \*out_num, float \*out_strength/g' header.h
sed -i 's/void compression(double \*out/void compression(float \*out/g' header.h
sed -i 's/bool gpu_permutation_testing(double \*gpu_runtime/bool gpu_permutation_testing(float \*gpu_runtime/g' header.h

# Update implementation files
sed -i 's/double gpu_runtime/float gpu_runtime/g' permutation_testing.cpp

# Make the script executable
chmod +x update_signatures.sh
