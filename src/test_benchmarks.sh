#!/bin/bash

# Test compiled benchmarks
source /opt/intel/oneapi/setvars.sh --quiet 2>&1 > /dev/null

BASEDIR="/home/stevens/HeCBench/src"
TESTLOG="$BASEDIR/test_results_$(date +%Y%m%d_%H%M%S).txt"

cd "$BASEDIR"

# Test a sample of successfully compiled benchmarks
BENCHMARKS="atan2-sycl accuracy-sycl adam-sycl aobench-sycl binomial-sycl black-scholes-sycl"

echo "Testing Sample SYCL Benchmarks" | tee "$TESTLOG"
echo "==============================" | tee -a "$TESTLOG"
echo "" | tee -a "$TESTLOG"

PASS=0
FAIL=0

for bench in $BENCHMARKS; do
    echo -n "Testing $bench... " | tee -a "$TESTLOG"
    
    if [ ! -f "$bench/main" ]; then
        echo "SKIP (no binary)" | tee -a "$TESTLOG"
        continue
    fi
    
    cd "$bench"
    
    # Try to run with common arguments
    if timeout 30 ./main 100 > /dev/null 2>&1; then
        echo "PASS" | tee -a "$TESTLOG"
        ((PASS++))
    elif timeout 30 ./main > /dev/null 2>&1; then
        echo "PASS (no args)" | tee -a "$TESTLOG"
        ((PASS++))
    else
        echo "FAIL (runtime error)" | tee -a "$TESTLOG"
        ((FAIL++))
    fi
    
    cd "$BASEDIR"
done

echo "" | tee -a "$TESTLOG"
echo "Test Results: PASS=$PASS FAIL=$FAIL" | tee -a "$TESTLOG"
echo "Full log: $TESTLOG"
