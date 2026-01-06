#!/bin/bash

# Log files
LOG_FILE="build_results.log"
ERROR_LOG="build_errors.log"
SUMMARY_FILE="build_summary.txt"
PROGRESS_FILE=".build_progress"

echo "Starting builds at $(date)" > "$LOG_FILE"
echo "Build errors log" > "$ERROR_LOG"
echo "Build Summary ($(date))" > "$SUMMARY_FILE"

# Statistics
total=0
success=0
failed=0
skipped=0
fp64_detected=0

# Function to modify Makefile
modify_makefile() {
    local makefile="$1"
    cp "$makefile" "${makefile}.backup"
    sed -i \
        -e 's/CC\s*=\s*clang++/CC        = icpx/' \
        -e 's/CUDA\s*=\s*no/CUDA      = no\nINTEL_GPU = yes/' \
        -e '/ifeq ($(GPU),yes)/i\ifeq ($(INTEL_GPU), yes)\n  CFLAGS += -fsycl-targets=spir64_gen -Xsycl-target-backend "-device acm-g10"\nendif' \
        "$makefile"
}

# Function to check for actual double precision computations
check_doubles() {
    local dir="$1"
    # Look for actual double precision computations, not just declarations
    grep -r -l "double" "$dir" 2>/dev/null | grep -v "Makefile\|\.backup\|\.txt\|\.md" | while read -r file; do
        # Check if the file contains actual double precision computations
        if grep -q "double.*=\|double.*(\|double.*{" "$file"; then
            echo "$file"
        fi
    done
}

# Load progress if exists
declare -A processed_dirs
if [ -f "$PROGRESS_FILE" ]; then
    while read -r dir; do
        processed_dirs["$dir"]=1
    done < "$PROGRESS_FILE"
fi

# Process each directory
for dir in *-sycl/; do
    if [ ! -d "$dir" ]; then
        continue
    fi

    # Skip if already processed
    if [ "${processed_dirs[$dir]}" = "1" ]; then
        echo "Skipping already processed: $dir"
        ((skipped++))
        continue
    fi

    ((total++))
    echo "Processing $dir" | tee -a "$LOG_FILE"
    cd "$dir" || continue

    # Check if Makefile exists
    if [ ! -f "Makefile" ]; then
        echo "No Makefile found in $dir" | tee -a "$ERROR_LOG"
        echo "$dir: No Makefile" >> "$SUMMARY_FILE"
        cd ..
        continue
    fi

    # Check for double precision usage
    doubles=$(check_doubles ".")
    if [ ! -z "$doubles" ]; then
        echo "Warning: Double precision found in $dir" | tee -a "$ERROR_LOG"
        echo "$doubles" >> "$ERROR_LOG"
        ((fp64_detected++))
    fi

    # Modify Makefile
    modify_makefile "Makefile"

    # Attempt to build
    echo "Building $dir..." | tee -a "$LOG_FILE"
    make clean >/dev/null 2>&1
    if make &> build.log; then
        echo "✓ Build successful: $dir" | tee -a "$LOG_FILE"
        echo "$dir: Success" >> "$SUMMARY_FILE"
        ((success++))
    else
        echo "✗ Build failed: $dir" | tee -a "$ERROR_LOG"
        echo "Error log for $dir:" >> "$ERROR_LOG"
        cat build.log >> "$ERROR_LOG"
        echo "$dir: Failed" >> "$SUMMARY_FILE"
        ((failed++))
    fi

    # Mark as processed
    echo "$dir" >> "$PROGRESS_FILE"
    cd ..
done

# Print summary
echo -e "\nBuild Summary:" | tee -a "$SUMMARY_FILE"
echo "Total directories: $total" | tee -a "$SUMMARY_FILE"
echo "Successful builds: $success" | tee -a "$SUMMARY_FILE"
echo "Failed builds: $failed" | tee -a "$SUMMARY_FILE"
echo "Skipped (already processed): $skipped" | tee -a "$SUMMARY_FILE"
echo "Double precision detected: $fp64_detected" | tee -a "$SUMMARY_FILE"

echo "Build process completed at $(date)" | tee -a "$LOG_FILE"
echo "See $LOG_FILE, $ERROR_LOG, and $SUMMARY_FILE for details"
