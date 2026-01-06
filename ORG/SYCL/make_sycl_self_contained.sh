#!/bin/bash

# Make SYCL benchmarks self-contained by copying reference headers locally
# and removing CUDA include paths

SCRIPT_DIR="/home/stevens/HeCBench/ORG/SYCL"
cd "$SCRIPT_DIR" || exit 1

FIXED=0
TOTAL=0

echo "========================================="
echo "Making SYCL Benchmarks Self-Contained"
echo "========================================="
echo ""

for bench in $(cat errors_wrong_compiler.txt); do
    if [ ! -d "$bench" ]; then
        continue
    fi

    TOTAL=$((TOTAL + 1))

    # Extract benchmark name without -sycl suffix
    base_name="${bench%-sycl}"

    # Look for CUDA version with reference.h
    cuda_dir="../CUDA/${base_name}-cuda"

    # Check if main.cpp includes reference.h
    if ! grep -q '#include "reference.h"' "$bench/main.cpp" 2>/dev/null; then
        continue
    fi

    # Check if reference.h exists in CUDA directory
    if [ ! -f "$cuda_dir/reference.h" ]; then
        echo "⚠️  $bench - no reference.h found in $cuda_dir"
        continue
    fi

    # Copy reference.h locally
    cp "$cuda_dir/reference.h" "$bench/"

    # Remove CUDA include paths from Makefile
    if [ -f "$bench/Makefile" ]; then
        # Backup
        cp "$bench/Makefile" "$bench/Makefile.bak_selfcontained"

        # Remove -I../xxx-cuda patterns
        sed -i 's/-I\.\.\/'${base_name}'-cuda//g' "$bench/Makefile"

        echo "✅ $bench - copied reference.h and removed CUDA include path"
        FIXED=$((FIXED + 1))
    fi
done

echo ""
echo "========================================="
echo "Made $FIXED/$TOTAL benchmarks self-contained"
echo "========================================="
echo ""
