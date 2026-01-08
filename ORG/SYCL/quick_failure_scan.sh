#!/bin/bash
source /opt/intel/oneapi/setvars.sh --force > /dev/null 2>&1

cd /home/stevens/HeCBench/ORG/SYCL

echo "Quick Failure Scan"
echo "==================="
echo

rm -f failure_analysis.txt failed_benchmarks.txt

success_count=0
fail_count=0
total_count=0

for bench in *-sycl; do
  if [ -d "$bench" ] && [ -f "$bench/Makefile" ]; then
    ((total_count++))
    
    # Quick check: does main executable exist from previous builds?
    if [ -f "$bench/main" ]; then
      ((success_count++))
      continue
    fi
    
    # Not compiled yet, try to compile
    cd "$bench"
    if timeout 60s make > compilation_logs/${bench}_test.log 2>&1; then
      ((success_count++))
    else
      ((fail_count++))
      # Capture error for categorization
      first_error=$(grep -E "error:|Error|fatal" compilation_logs/${bench}_test.log 2>/dev/null | head -1)
      echo "$bench" >> ../failed_benchmarks.txt
      echo "$bench|$first_error" >> ../failure_analysis.txt
      echo "✗ $bench"
    fi
    cd ..
    
    # Progress indicator
    if [ $((total_count % 50)) -eq 0 ]; then
      echo "Progress: $total_count tested, $fail_count failures so far..."
    fi
  fi
done

echo
echo "========================================="
echo "Results: $success_count/$total_count working ($(echo "scale=1; $success_count*100/$total_count" | bc)%)"
echo "Failed: $fail_count"
echo "========================================="
