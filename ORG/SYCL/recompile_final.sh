#!/bin/bash

# Final recompile after all fixes

SCRIPT_DIR="/home/stevens/HeCBench/ORG/SYCL"
cd "$SCRIPT_DIR" || exit 1

source /opt/intel/oneapi/setvars.sh --force > /dev/null 2>&1

SUCCESS=0
FAILED=0
TOTAL=0

echo "========================================="
echo "Final Recompilation"
echo "========================================="
echo ""

for bench in $(cat errors_wrong_compiler.txt); do
    if [ ! -d "$bench" ]; then
        continue
    fi

    TOTAL=$((TOTAL + 1))
    echo -n "[$TOTAL/87] $bench... "

    cd "$bench" || continue
    make clean > /dev/null 2>&1

    if timeout 60s make > "../compilation_logs/${bench}_final2.log" 2>&1; then
        if [ -f "main" ]; then
            echo "✅"
            SUCCESS=$((SUCCESS + 1))
        else
            echo "❌"
            FAILED=$((FAILED + 1))
        fi
    else
        echo "❌"
        FAILED=$((FAILED + 1))
    fi

    cd "$SCRIPT_DIR" || exit 1
done

# Count total working now
TOTAL_WORKING=$(find . -maxdepth 2 -name "main" -executable -path "./*-sycl/*" | wc -l)

echo ""
echo "========================================="
echo "FINAL RESULTS"
echo "========================================="
echo "Attempted:         $TOTAL"
echo "Successful:        $SUCCESS"
echo "Failed:            $FAILED"
echo ""
echo "📊 Overall Progress:"
echo "   Previously:     256/444 (57.7%)"
echo "   Now:            $TOTAL_WORKING/444 ($(awk "BEGIN {printf \"%.1f\", ($TOTAL_WORKING/444)*100}")%)"
echo "   Improvement:    +$((TOTAL_WORKING - 256)) benchmarks"
echo ""

if [ $SUCCESS -gt 0 ]; then
    echo "🎉 SUCCESS! Fixed $SUCCESS out of 87 benchmarks!"
fi
