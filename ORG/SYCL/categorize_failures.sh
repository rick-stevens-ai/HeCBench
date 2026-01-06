#!/bin/bash

echo "Analyzing build failures..."

# Create categories file
echo "Build Failure Analysis" > failure_analysis.txt
echo "======================" >> failure_analysis.txt
echo "" >> failure_analysis.txt

echo "1. SYCL Header Missing:" >> failure_analysis.txt
grep -r "fatal error: sycl/sycl.hpp" . 2>/dev/null | cut -d: -f1 | sort -u >> failure_analysis.txt

echo -e "\n2. Missing Compiler (clang++):" >> failure_analysis.txt
grep -r "make: clang++: No such file or directory" build_results.log | cut -d: -f1 | sort -u >> failure_analysis.txt

echo -e "\n3. Double Precision Issues:" >> failure_analysis.txt
grep -r "Double type is not supported" build_results.log | cut -d: -f1 | sort -u >> failure_analysis.txt

echo -e "\n4. Missing Dependencies:" >> failure_analysis.txt
grep -r "fatal error:.* No such file or directory" . 2>/dev/null | grep -v "sycl/sycl.hpp" | cut -d: -f1 | sort -u >> failure_analysis.txt

echo -e "\n5. Successfully Built Examples:" >> failure_analysis.txt
grep "SUCCESS" build_summary.txt | cut -d: -f1 >> failure_analysis.txt

echo "Analysis written to failure_analysis.txt"
cat failure_analysis.txt
