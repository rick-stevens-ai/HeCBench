#!/bin/bash

# Batch compile SYCL benchmarks from src directory
source /opt/intel/oneapi/setvars.sh --quiet 2>&1 > /dev/null

BASEDIR="/home/stevens/HeCBench/src"
LOGDIR="$BASEDIR/compilation_logs_$(date +%Y%m%d_%H%M%S)"
mkdir -p "$LOGDIR"

echo "Starting batch compilation from src..."
echo "Log directory: $LOGDIR"

cd "$BASEDIR"

# Get list of all SYCL benchmarks
BENCHMARKS=$(ls -d *-sycl | head -50)

PASS=0
FAIL=0
SKIP=0

for bench in $BENCHMARKS; do
    echo -n "[$((PASS + FAIL + 1))] $bench... "
    
    if [ ! -d "$BASEDIR/$bench" ]; then
        echo "SKIP (no directory)"
        ((SKIP++))
        continue
    fi
    
    if [ ! -f "$BASEDIR/$bench/Makefile" ]; then
        echo "SKIP (no Makefile)"
        ((SKIP++))
        continue
    fi
    
    # Try compilation
    (cd "$BASEDIR/$bench" && make CC=icpx clean > /dev/null 2>&1)
    
    if (cd "$BASEDIR/$bench" && timeout 90 make CC=icpx > "$LOGDIR/${bench}.log" 2>&1); then
        if [ -f "$BASEDIR/$bench/main" ]; then
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

echo ""
echo "============================="
echo "Batch Compilation Complete"
echo "============================="
echo "PASS: $PASS"
echo "FAIL: $FAIL"
echo "SKIP: $SKIP"
echo "TOTAL: $((PASS + FAIL + SKIP))"
echo "Logs: $LOGDIR"
