#!/bin/bash
source /opt/intel/oneapi/setvars.sh --force > /dev/null 2>&1

cd /home/stevens/HeCBench/ORG/SYCL

echo "Testing Sample of Benchmarks Without Main"
echo "==========================================="
echo

mkdir -p compilation_logs

# Test first 20 from the list
head -20 benchmarks_without_main.txt | while read bench; do
  echo -n "Testing $bench... "
  cd "$bench"
  make clean > /dev/null 2>&1
  if timeout 90s make > ../compilation_logs/${bench}_test.log 2>&1; then
    echo "✓ SUCCESS"
  else
    echo "✗ FAILED"
    # Show first error
    grep -E "error:" ../compilation_logs/${bench}_test.log 2>/dev/null | head -1
  fi
  cd ..
done
