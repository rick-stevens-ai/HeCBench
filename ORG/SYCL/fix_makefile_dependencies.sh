#!/bin/bash

# Script to fix Makefile %.o rules with missing dependencies
# These cause make to fall back to g++ instead of using $(CC)

SCRIPT_DIR="/home/stevens/HeCBench/ORG/SYCL"
cd "$SCRIPT_DIR" || exit 1

FIXED=0
SKIPPED=0

echo "========================================="
echo "Fixing Makefile Dependency Issues"
echo "========================================="
echo ""

for bench in $(cat errors_wrong_compiler.txt); do
    makefile="$bench/Makefile"

    if [ ! -f "$makefile" ]; then
        echo "⚠️  $bench: Makefile not found"
        SKIPPED=$((SKIPPED + 1))
        continue
    fi

    # Backup
    cp "$makefile" "$makefile.bak_deps"

    # Check if the Makefile has a %.o rule with dependencies like ../benchmark-cuda/
    if grep -q "^%.o.*:.*%.cpp.*\.\./.*-cuda" "$makefile"; then
        echo "Fixing $bench..."

        # Replace the rule to have only .cpp as prerequisite (dependencies become order-only)
        # This ensures $(CC) is used, not g++
        sed -i 's|^\(%.o\): \(%.cpp\).*$|\1: \2\n\t$(CC) $(CFLAGS) -c $< -o $@|' "$makefile"

        # Check if it actually changed
        if ! diff -q "$makefile" "$makefile.bak_deps" > /dev/null 2>&1; then
            echo "  ✅ $bench: Fixed"
            FIXED=$((FIXED + 1))
        else
            rm "$makefile.bak_deps"
        fi
    else
        rm "$makefile.bak_deps"
    fi
done

echo ""
echo "========================================="
echo "Summary"
echo "========================================="
echo "Benchmarks fixed:    $FIXED"
echo "Skipped:             $SKIPPED"
echo "Total:               $(wc -l < errors_wrong_compiler.txt)"
echo ""

if [ $FIXED -gt 0 ]; then
    echo "✅ Makefiles have been fixed!"
    echo ""
    echo "Backups saved as Makefile.bak_deps in each directory"
    echo ""
    echo "Next step: Recompile the fixed benchmarks"
    echo "  ./recompile_fixed.sh"
fi
