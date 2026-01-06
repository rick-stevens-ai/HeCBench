#!/bin/bash

# Script to recompile benchmarks that were using g++ instead of icpx
# Uses -r flag to disable make's built-in rules

SCRIPT_DIR="/home/stevens/HeCBench/ORG/SYCL"
cd "$SCRIPT_DIR" || exit 1

source /opt/intel/oneapi/setvars.sh --force > /dev/null 2>&1

SUCCESS=0
FAILED=0
TOTAL=0

echo "========================================="
echo "Recompiling Wrong Compiler Benchmarks"
echo "========================================="
echo ""
echo "Strategy: Using make -r to disable built-in rules"
echo ""

for bench in $(cat errors_wrong_compiler.txt); do
    if [ ! -d "$bench" ]; then
        continue
    fi

    TOTAL=$((TOTAL + 1))
    echo -n "[$TOTAL/$(($(wc -l < errors_wrong_compiler.txt)))] $bench... "

    cd "$bench" || continue

    # Clean first
    make clean > /dev/null 2>&1

    # Compile with -r to disable built-in rules, forcing use of Makefile rules
    # Also set CC=icpx explicitly
    if timeout 60s make -r CC=icpx > "../compilation_logs/${bench}_fixed.log" 2>&1; then
        if [ -f "main" ]; then
            echo "✅ SUCCESS"
            SUCCESS=$((SUCCESS + 1))
        else
            echo "❌ FAILED (no executable)"
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
echo "Total attempted:   $TOTAL"
echo "Successful:        $SUCCESS"
echo "Failed:            $FAILED"
echo "Success rate:      $(awk "BEGIN {printf \"%.1f\", ($SUCCESS/$TOTAL)*100}")%"
echo ""

# Count total working now
TOTAL_WORKING=$(find . -maxdepth 2 -name "main" -executable -path "./*-sycl/*" | wc -l)
TOTAL_BENCHES=444

echo "📊 Overall Progress"
echo "   Previously working: 256/444 (57.7%)"
echo "   Now working:        $TOTAL_WORKING/444 ($(awk "BEGIN {printf \"%.1f\", ($TOTAL_WORKING/$TOTAL_BENCHES)*100}")%)"
echo "   Improvement:        +$((TOTAL_WORKING - 256)) benchmarks"
echo ""

if [ $SUCCESS -gt 0 ]; then
    echo "✅ $SUCCESS benchmarks now compile!"
fi

if [ $FAILED -gt 0 ]; then
    echo "⚠️  $FAILED benchmarks still have issues"
    echo "   Check logs in compilation_logs/*_fixed.log"
fi
