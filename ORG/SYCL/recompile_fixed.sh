#!/bin/bash

# Script to recompile all fixed benchmarks

SCRIPT_DIR="/home/stevens/HeCBench/ORG/SYCL"
cd "$SCRIPT_DIR" || exit 1

source /opt/intel/oneapi/setvars.sh --force > /dev/null 2>&1

SUCCESS=0
FAILED=0
TOTAL=0

echo "========================================="
echo "Recompiling Fixed Benchmarks"
echo "========================================="
echo ""

# Recompile benchmarks from wrong_compiler list
for bench in $(cat errors_wrong_compiler.txt); do
    TOTAL=$((TOTAL + 1))

    if [ ! -d "$bench" ]; then
        continue
    fi

    echo -n "[$TOTAL] $bench... "

    cd "$bench" || continue

    # Clean and compile
    make clean > /dev/null 2>&1
    if timeout 60s make CC=icpx > "../compilation_logs/${bench}_retry.log" 2>&1; then
        if [ -f "main" ]; then
            echo "✅ SUCCESS"
            SUCCESS=$((SUCCESS + 1))
        else
            echo "❌ FAILED (no executable)"
            FAILED=$((FAILED + 1))
        fi
    else
        echo "❌ FAILED (compilation error)"
        FAILED=$((FAILED + 1))
    fi

    cd "$SCRIPT_DIR" || exit 1
done

# Also recompile FP64 issues (if already converted)
echo ""
echo "Checking FP64 benchmarks..."

for bench in $(cat errors_fp64.txt 2>/dev/null | cut -d: -f2); do
    if [ -f "$bench/main" ]; then
        echo "✓ $bench: Already compiled"
        continue
    fi

    if [ ! -d "$bench" ]; then
        continue
    fi

    TOTAL=$((TOTAL + 1))
    echo -n "[$TOTAL] $bench... "

    cd "$bench" || continue

    make clean > /dev/null 2>&1
    if timeout 60s make CC=icpx > "../compilation_logs/${bench}_retry.log" 2>&1; then
        if [ -f "main" ]; then
            echo "✅ SUCCESS"
            SUCCESS=$((SUCCESS + 1))
        else
            echo "❌ FAILED"
            FAILED=$((FAILED + 1))
        fi
    else
        echo "❌ FAILED"
        FAILED=$((FAILED + 1))
    fi

    cd "$SCRIPT_DIR" || exit 1
done

echo ""
echo "========================================="
echo "Recompilation Summary"
echo "========================================="
echo "Attempted:     $TOTAL"
echo "Successful:    $SUCCESS"
echo "Failed:        $FAILED"
echo ""

# Count total working now
TOTAL_WORKING=$(find . -maxdepth 2 -name "main" -executable -path "./*-sycl/*" | wc -l)
echo "📊 Total working benchmarks: $TOTAL_WORKING/444"
echo "   Success rate: $(awk "BEGIN {printf \"%.1f\", ($TOTAL_WORKING/444)*100}")%"
