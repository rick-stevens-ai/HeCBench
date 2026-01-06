#!/bin/bash

# Script to recompile benchmarks after SYCL 2020 API update

SCRIPT_DIR="/home/stevens/HeCBench/ORG/SYCL"
cd "$SCRIPT_DIR" || exit 1

source /opt/intel/oneapi/setvars.sh --force > /dev/null 2>&1

SUCCESS=0
FAILED=0
TOTAL=0
ALREADY_WORKING=0

echo "========================================="
echo "Recompiling SYCL 2020 Updated Benchmarks"
echo "========================================="
echo ""

for bench in $(cat errors_deprecated_api.txt); do
    if [ ! -d "$bench" ]; then
        continue
    fi

    TOTAL=$((TOTAL + 1))

    # Check if already has executable
    if [ -f "$bench/main" ]; then
        echo "[$TOTAL] $bench: Already has executable, recompiling..."
        cd "$bench" && make clean > /dev/null 2>&1
        cd "$SCRIPT_DIR"
    fi

    echo -n "[$TOTAL] $bench... "

    cd "$bench" || continue

    # Try to compile
    if timeout 60s make CC=icpx > "../compilation_logs/${bench}_sycl2020.log" 2>&1; then
        if [ -f "main" ]; then
            echo "✅ SUCCESS"
            SUCCESS=$((SUCCESS + 1))
        else
            echo "❌ FAILED (no executable)"
            FAILED=$((FAILED + 1))
            # Show last few lines of error
            echo "    Last errors:"
            tail -5 "../compilation_logs/${bench}_sycl2020.log" | sed 's/^/    /'
        fi
    else
        echo "❌ FAILED (compilation error)"
        FAILED=$((FAILED + 1))
        echo "    Check log: compilation_logs/${bench}_sycl2020.log"
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
echo ""

# Count total working now
TOTAL_WORKING=$(find . -maxdepth 2 -name "main" -executable -path "./*-sycl/*" | wc -l)
echo "📊 Total working benchmarks: $TOTAL_WORKING/444"
echo "   Success rate: $(awk "BEGIN {printf \"%.1f\", ($TOTAL_WORKING/444)*100}")%"
echo ""

if [ $SUCCESS -gt 0 ]; then
    echo "✅ $SUCCESS benchmarks now compile with SYCL 2020 API!"
    echo ""
    echo "Updated from: 256/444 (57.7%)"
    echo "         to: $TOTAL_WORKING/444 ($(awk "BEGIN {printf \"%.1f\", ($TOTAL_WORKING/444)*100}")%)"
fi

if [ $FAILED -gt 0 ]; then
    echo ""
    echo "⚠️  $FAILED benchmarks still have issues"
    echo "Check logs in compilation_logs/*_sycl2020.log"
fi
