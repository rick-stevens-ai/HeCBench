#!/bin/bash

# Test a sample of compiled benchmarks
source /opt/intel/oneapi/setvars.sh --quiet 2>&1 > /dev/null

BASEDIR="/home/stevens/HeCBench/src"
cd "$BASEDIR"

echo "Testing Sample of Compiled SYCL Benchmarks"
echo "==========================================="
echo ""

# Select a diverse sample
BENCHMARKS="atan2-sycl binomial-sycl black-scholes-sycl matrixT-sycl triad-sycl babelstream-sycl attentionMultiHead-sycl kmeans-sycl mandelbrot-sycl srad-sycl"

PASS=0
FAIL=0
SKIP=0

for bench in $BENCHMARKS; do
    printf "%-30s ... " "$bench"
    
    if [ ! -f "$bench/main" ]; then
        echo "SKIP (no binary)"
        ((SKIP++))
        continue
    fi
    
    cd "$bench"
    
    # Try different common argument patterns
    if timeout 20 ./main 100 > /tmp/test_out.txt 2>&1; then
        echo "PASS"
        ((PASS++))
    elif timeout 20 ./main 10000000 100 > /tmp/test_out.txt 2>&1; then
        echo "PASS"
        ((PASS++))
    elif timeout 20 ./main > /tmp/test_out.txt 2>&1; then
        if grep -qi "usage\|error" /tmp/test_out.txt; then
            echo "FAIL (needs args)"
            ((FAIL++))
        else
            echo "PASS (no args)"
            ((PASS++))
        fi
    else
        echo "FAIL (runtime error)"
        ((FAIL++))
    fi
    
    cd "$BASEDIR"
done

echo ""
echo "Test Results: PASS=$PASS FAIL=$FAIL SKIP=$SKIP"
echo ""
