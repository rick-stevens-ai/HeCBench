#!/bin/bash

# Log files
LOG_FILE="build_results.log"
ERROR_LOG="build_errors.log"
SUMMARY_FILE="build_summary.txt"
PROGRESS_FILE=".build_progress"

# Create PASSED and FAILED directories
mkdir -p PASSED FAILED

echo "Starting builds at $(date)" > "$LOG_FILE"
echo "Build errors log" > "$ERROR_LOG"
echo "Build Summary ($(date))" > "$SUMMARY_FILE"

# Statistics
total=0
success=0
failed=0
skipped=0
fp64_detected=0
run_success=0
run_failed=0

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
    grep -r -l "double" "$dir" 2>/dev/null | grep -v "Makefile\|\.backup\|\.txt\|\.md" | while read -r file; do
        if grep -q "double.*=\|double.*(\|double.*{" "$file"; then
            echo "$file"
        fi
    done
}

# Function to try running the built example
try_run() {
    local dir="$1"
    local executable
    
    # Find the most likely executable (non-object file that's executable)
    executable=$(find . -type f -executable -not -name "*.o" -not -name "*.so" | head -n 1)
    
    if [ -z "$executable" ]; then
        echo "No executable found in $dir" | tee -a "$LOG_FILE"
        return 1
    fi

    echo "Attempting to run $executable..." | tee -a "$LOG_FILE"
    timeout 30s $executable &> run.log
    local status=$?

    if [ $status -eq 0 ]; then
        echo "✓ Run successful: $dir" | tee -a "$LOG_FILE"
        return 0
    elif [ $status -eq 124 ]; then
        echo "✗ Run timed out: $dir" | tee -a "$ERROR_LOG"
        echo "Program timed out after 30 seconds" >> "$ERROR_LOG"
        return 1
    else
        echo "✗ Run failed: $dir" | tee -a "$ERROR_LOG"
        cat run.log >> "$ERROR_LOG"
        return 1
    fi
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
    if [ ! -d "$dir" ] || [[ "$dir" == "PASSED/"* ]] || [[ "$dir" == "FAILED/"* ]]; then
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
        echo "$dir: Build Success" >> "$SUMMARY_FILE"
        ((success++))

        # Try to run the example
        if try_run "$dir"; then
            ((run_success++))
            echo "$dir: Run Success" >> "$SUMMARY_FILE"
            cd ..
            echo "Moving $dir to PASSED/"
            mv "$dir" "PASSED/"
        else
            ((run_failed++))
            echo "$dir: Run Failed" >> "$SUMMARY_FILE"
            cd ..
        fi
    else
        echo "✗ Build failed: $dir" | tee -a "$ERROR_LOG"
        echo "Error log for $dir:" >> "$ERROR_LOG"
        cat build.log >> "$ERROR_LOG"
        echo "$dir: Build Failed" >> "$SUMMARY_FILE"
        ((failed++))
        cd ..
    fi

    # Mark as processed
    echo "$dir" >> "$PROGRESS_FILE"
done

# Print summary
echo -e "\nBuild Summary:" | tee -a "$SUMMARY_FILE"
echo "Total directories: $total" | tee -a "$SUMMARY_FILE"
echo "Successful builds: $success" | tee -a "$SUMMARY_FILE"
echo "Failed builds: $failed" | tee -a "$SUMMARY_FILE"
echo "Successful runs: $run_success" | tee -a "$SUMMARY_FILE"
echo "Failed runs: $run_failed" | tee -a "$SUMMARY_FILE"
echo "Skipped (already processed): $skipped" | tee -a "$SUMMARY_FILE"
echo "Double precision detected: $fp64_detected" | tee -a "$SUMMARY_FILE"

echo "Build process completed at $(date)" | tee -a "$LOG_FILE"
echo "See $LOG_FILE, $ERROR_LOG, and $SUMMARY_FILE for details"
