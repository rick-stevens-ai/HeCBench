#!/bin/bash

# Final comprehensive Makefile fix
# Simplifies %.o: %.cpp rules to remove non-existent dependencies

SCRIPT_DIR="/home/stevens/HeCBench/ORG/SYCL"
cd "$SCRIPT_DIR" || exit 1

FIXED=0

echo "========================================="
echo "Fixing Makefiles - Final Approach"
echo "========================================="
echo ""

for bench in $(cat errors_wrong_compiler.txt); do
    makefile="$bench/Makefile"

    if [ ! -f "$makefile" ]; then
        continue
    fi

    # Backup
    cp "$makefile" "$makefile.bak_final"

    # Find and simplify the %.o: %.cpp rule
    # Remove all dependencies except %.cpp, keep the recipe
    if grep -q "^%.o.*:.*%.cpp" "$makefile"; then
        # Use awk to properly handle multi-line rules
        awk '
        /^%.o.*:.*%.cpp/ {
            print "%.o: %.cpp"
            print "\t$(CC) $(CFLAGS) -c $< -o $@"
            # Skip the old recipe line (next line starting with tab)
            getline
            next
        }
        {print}
        ' "$makefile" > "$makefile.tmp"

        mv "$makefile.tmp" "$makefile"

        echo "✅ $bench"
        FIXED=$((FIXED + 1))
    else
        rm "$makefile.bak_final"
    fi
done

echo ""
echo "========================================="
echo "Fixed $FIXED Makefiles"
echo "========================================="
echo ""
echo "Now recompiling..."
echo ""

# Now recompile
SUCCESS=0
FAILED=0
TOTAL=0

source /opt/intel/oneapi/setvars.sh --force > /dev/null 2>&1

for bench in $(cat errors_wrong_compiler.txt); do
    if [ ! -d "$bench" ]; then
        continue
    fi

    TOTAL=$((TOTAL + 1))
    echo -n "[$TOTAL/87] $bench... "

    cd "$bench" || continue
    make clean > /dev/null 2>&1

    if timeout 60s make > "../compilation_logs/${bench}_final.log" 2>&1; then
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

# Final stats
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
    echo "🎉 SUCCESS! Fixed $SUCCESS benchmarks!"
fi
