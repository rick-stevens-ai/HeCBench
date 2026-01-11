#!/bin/bash
source /opt/intel/oneapi/setvars.sh --force > /dev/null 2>&1
cd /home/stevens/HeCBench/ORG/SYCL

# List of known failing benchmarks
benchmarks=(
  "bsw" "shuffle" "btree"           # SYCL API issues
  "quicksort" "remap" "sc"          # Template issues
  "leukocyte" "lulesh" "dwt2d"      # Undeclared identifiers
  "frechet" "complex" "permute"      # Algorithm issues
  "opticalFlow" "kmeans" "si"       # Build/other issues
  "convolutionDeformable" "segment-reduce" "tensorT"
)

echo "Testing Remaining Benchmarks"
echo "============================="
echo

for bench in "${benchmarks[@]}"; do
  bench_dir="${bench}-sycl"
  if [ -d "$bench_dir" ] && [ -f "$bench_dir/Makefile" ]; then
    echo "=== $bench_dir ==="
    cd "$bench_dir"
    make clean > /dev/null 2>&1
    timeout 30s make > ../test_logs/${bench}_test.log 2>&1
    if [ $? -eq 0 ]; then
      echo "✓ SUCCESS"
    else
      # Show first error
      grep -E "error:" ../test_logs/${bench}_test.log | head -1 | cut -c1-100
    fi
    cd ..
  fi
done
