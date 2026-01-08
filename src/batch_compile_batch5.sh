#!/bin/bash

# Batch 5: remaining benchmarks (351+)
source /opt/intel/oneapi/setvars.sh --quiet 2>&1 > /dev/null

BASEDIR="/home/stevens/HeCBench/src"
LOGDIR="$BASEDIR/compilation_logs_20260108_102500"

cd "$BASEDIR"

BENCHMARKS=$(ls -d *-sycl | tail -n +351)

PASS=0
FAIL=0
SKIP=0

for bench in $BENCHMARKS; do
    echo -n "[$((PASS + FAIL + 351))] $bench... "
    
    if [ ! -f "$BASEDIR/$bench/Makefile" ]; then
        echo "SKIP (no Makefile)"
        ((SKIP++))
        continue
    fi
    
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
echo "Batch 5: PASS=$PASS FAIL=$FAIL SKIP=$SKIP"
