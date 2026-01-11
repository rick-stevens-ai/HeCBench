#!/bin/bash

# Batch compile SYCL benchmarks
source /opt/intel/oneapi/setvars.sh --quiet 2>&1 > /dev/null

BASEDIR="/home/stevens/HeCBench/ORG/SYCL"
LOGDIR="$BASEDIR/compilation_logs_$(date +%Y%m%d_%H%M%S)"
mkdir -p "$LOGDIR"

echo "Starting batch compilation..."
echo "Log directory: $LOGDIR"

# Get list of FAIL_COMPILE benchmarks
BENCHMARKS=$(grep "FAIL_COMPILE" compilation_summary.txt | cut -d: -f1 | head -20)

PASS=0
FAIL=0

for bench in $BENCHMARKS; do
    echo -n "Compiling $bench... "
    
    if [ ! -d "$bench" ]; then
        echo "SKIP (no directory)"
        continue
    fi
    
    cd "$BASEDIR/$bench"
    
    # Try compilation
    make CC=icpx clean > /dev/null 2>&1
    if timeout 60 make CC=icpx > "$LOGDIR/${bench}.log" 2>&1; then
        if [ -f "main" ]; then
            echo "SUCCESS"
            ((PASS++))
        else
            echo "FAIL (no binary)"
            ((FAIL++))
        fi
    else
        echo "FAIL (compilation error)"
        ((FAIL++))
    fi
done

cd "$BASEDIR"

echo ""
echo "============================="
echo "Batch Compilation Complete"
echo "============================="
echo "PASS: $PASS"
echo "FAIL: $FAIL"
echo "Logs: $LOGDIR"
