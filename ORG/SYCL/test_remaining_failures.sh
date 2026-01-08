#!/bin/bash
source /opt/intel/oneapi/setvars.sh --force > /dev/null 2>&1

cd /home/stevens/HeCBench/ORG/SYCL

echo "Testing Remaining Benchmarks"
echo "=============================="
echo

# Test remaining from line 21 onwards
tail -n +21 benchmarks_without_main.txt | while read bench; do
  echo -n "Testing $bench... "
  cd "$bench"
  make clean > /dev/null 2>&1
  if timeout 90s make > ../compilation_logs/${bench}_test.log 2>&1; then
    echo "✓ SUCCESS"
    echo "$bench" >> ../newly_working.txt
  else
    echo "✗ FAILED"
    echo "$bench" >> ../still_failing.txt
    # Capture first error for categorization
    first_error=$(grep -E "error:|fatal error:" ../compilation_logs/${bench}_test.log 2>/dev/null | head -1)
    echo "$bench|$first_error" >> ../failure_categories.txt
  fi
  cd ..
done

echo
echo "Summary written to newly_working.txt and still_failing.txt"
