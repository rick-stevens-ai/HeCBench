#!/bin/bash

# Fix Makefiles that have CC = clang++ instead of CC = icpx

SCRIPT_DIR="/home/stevens/HeCBench/ORG/SYCL"
cd "$SCRIPT_DIR" || exit 1

FIXED=0

echo "========================================="
echo "Fixing Compiler Variables in Makefiles"
echo "========================================="
echo ""

for bench in $(cat errors_wrong_compiler.txt); do
    makefile="$bench/Makefile"

    if [ ! -f "$makefile" ]; then
        continue
    fi

    # Check if it has CC = clang++
    if grep -q "^CC.*=.*clang++" "$makefile"; then
        # Backup
        cp "$makefile" "$makefile.bak_cc"

        # Change CC = clang++ to CC = icpx
        sed -i 's/^CC\s*=\s*clang++/CC        = icpx/' "$makefile"

        echo "✅ $bench"
        FIXED=$((FIXED + 1))
    fi
done

echo ""
echo "========================================="
echo "Fixed $FIXED Makefiles"
echo "========================================="
