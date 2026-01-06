#!/bin/bash

# Log file
LOG_FILE="build_results.log"
ERROR_LOG="build_errors.log"

echo "Starting builds at $(date)" > "$LOG_FILE"
echo "Build errors log" > "$ERROR_LOG"

# Function to modify Makefile
modify_makefile() {
    local makefile="$1"
    # Backup original Makefile
    cp "$makefile" "${makefile}.backup"
    
    # Update compiler and flags
    sed -i \
        -e 's/CC\s*=\s*clang++/CC        = icpx/' \
        -e 's/CUDA\s*=\s*no/CUDA      = no\nINTEL_GPU = yes/' \
        -e '/ifeq ($(GPU),yes)/i\ifeq ($(INTEL_GPU), yes)\n  CFLAGS += -fsycl-targets=spir64_gen -Xsycl-target-backend "-device acm-g10"\nendif' \
        "$makefile"
}

# Function to check for double precision
check_doubles() {
    local dir="$1"
    grep -r "double" "$dir" 2>/dev/null | grep -v "Makefile\|\.backup" || true
}

# Process each directory
for dir in *-sycl/; do
    if [ ! -d "$dir" ]; then
        continue
    fi

    echo "Processing $dir" | tee -a "$LOG_FILE"
    cd "$dir" || continue

    # Check if Makefile exists
    if [ ! -f "Makefile" ]; then
        echo "No Makefile found in $dir" | tee -a "$ERROR_LOG"
        cd ..
        continue
    fi

    # Check for double precision usage
    doubles=$(check_doubles ".")
    if [ ! -z "$doubles" ]; then
        echo "Warning: Double precision found in $dir" | tee -a "$ERROR_LOG"
        echo "$doubles" >> "$ERROR_LOG"
    fi

    # Modify Makefile
    modify_makefile "Makefile"

    # Attempt to build
    echo "Building $dir..." | tee -a "$LOG_FILE"
    make clean >/dev/null 2>&1
    if make &> build.log; then
        echo "✓ Build successful: $dir" | tee -a "$LOG_FILE"
    else
        echo "✗ Build failed: $dir" | tee -a "$ERROR_LOG"
        cat build.log >> "$ERROR_LOG"
    fi

    cd ..
done

echo "Build process completed at $(date)" | tee -a "$LOG_FILE"
echo "See $LOG_FILE and $ERROR_LOG for details"
