#!/bin/bash
source /opt/intel/oneapi/setvars.sh --force > /dev/null 2>&1
cd /home/stevens/HeCBench/ORG/SYCL

echo "=== Testing norm2-sycl ==="
cd norm2-sycl && make clean > /dev/null 2>&1
timeout 30s make 2>&1 | grep -E "error:" | head -3
cd ..

echo
echo "=== Testing depixel-sycl ==="
cd depixel-sycl && make clean > /dev/null 2>&1
timeout 30s make 2>&1 | grep -E "error:" | head -3
cd ..
