#!/bin/bash

# Continue batch compile from where we left off
source /opt/intel/oneapi/setvars.sh --quiet 2>&1 > /dev/null

BASEDIR="/home/stevens/HeCBench/src"
LOGDIR="$BASEDIR/compilation_logs_20260108_102500"

cd "$BASEDIR"

# Get benchmarks 51-150
BENCHMARKS=$(ls -d *-sycl | tail -n +51 | head -100)

PASS=0
FAIL=0
SKIP=0

for bench in $BENCHMARKS; do
    echo -n "[$((PASS + FAIL + 51))] $bench... "
    
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
echo "Batch 2 Compilation Complete"
echo "============================="
echo "PASS: $PASS"
echo "FAIL: $FAIL"
echo "SKIP: $SKIP"
