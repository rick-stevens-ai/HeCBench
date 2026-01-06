#!/bin/bash

# Copy all missing headers from CUDA directories to SYCL benchmarks

SCRIPT_DIR="/home/stevens/HeCBench/ORG/SYCL"
cd "$SCRIPT_DIR" || exit 1

FIXED=0

echo "========================================="
echo "Copying Missing Headers"
echo "========================================="
echo ""

# Get list of failed benchmarks
for bench in $(cat errors_wrong_compiler.txt); do
    if [ ! -d "$bench" ]; then
        continue
    fi

    # Check if main doesn't exist (compilation failed)
    if [ -f "$bench/main" ]; then
        continue
    fi

    # Extract benchmark name without -sycl suffix
    base_name="${bench%-sycl}"
    cuda_dir="../CUDA/${base_name}-cuda"

    # Check if CUDA directory exists
    if [ ! -d "$cuda_dir" ]; then
        continue
    fi

    # Find all header files included in main.cpp
    headers=$(grep '^#include "' "$bench/main.cpp" 2>/dev/null | sed 's/#include "\(.*\)"/\1/')

    copied=false
    for header in $headers; do
        # Skip if header already exists locally
        if [ -f "$bench/$header" ]; then
            continue
        fi

        # Try to copy from CUDA directory
        if [ -f "$cuda_dir/$header" ]; then
            cp "$cuda_dir/$header" "$bench/"
            echo "  Copied $header"
            copied=true
        fi
    done

    if [ "$copied" = true ]; then
        # Also remove CUDA include paths from Makefile
        if [ -f "$bench/Makefile" ]; then
            cp "$bench/Makefile" "$bench/Makefile.bak_headers"
            sed -i "s|-I\.\./\?${base_name}-cuda/?||g" "$bench/Makefile"
        fi
        echo "✅ $bench"
        FIXED=$((FIXED + 1))
    fi
done

echo ""
echo "========================================="
echo "Copied headers for $FIXED benchmarks"
echo "========================================="
