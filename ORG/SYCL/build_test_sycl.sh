#!/bin/bash

# Initialize log files
echo "Build Test Started: $(date)" > build_results.log
echo "Build Failures:" > build_errors.log
> build_summary.txt

# Initialize counters
total=0
success=0
failed=0

# Function to test building an example
test_build() {
    local dir=$1
    echo "Testing $dir..."
    echo "============================================" >> build_results.log
    echo "Testing $dir" >> build_results.log
    
    # Change to directory and attempt to build
    cd "$dir" 2>/dev/null
    if [ $? -ne 0 ]; then
        echo "ERROR: Could not cd to $dir" >> ../build_errors.log
        return 1
    fi
    
    # Check if Makefile exists
    if [ ! -f "Makefile" ]; then
        echo "ERROR: No Makefile in $dir" >> ../build_errors.log
        cd ..
        return 1
    fi
    
    # Attempt to build
    make clean >/dev/null 2>&1
    make 2>&1 | tee -a ../build_results.log
    
    if [ ${PIPESTATUS[0]} -eq 0 ]; then
        echo "$dir: SUCCESS" >> ../build_summary.txt
        cd ..
        return 0
    else
        echo "$dir: FAILED" >> ../build_summary.txt
        echo "Failed to build $dir" >> ../build_errors.log
        cd ..
        return 1
    fi
}

# Main loop
for dir in *-sycl/; do
    if [ -d "$dir" ]; then
        total=$((total + 1))
        if test_build "${dir%/}"; then
            success=$((success + 1))
        else
            failed=$((failed + 1))
        fi
    fi
done

# Generate summary
echo "============================================" >> build_summary.txt
echo "Build Test Summary" >> build_summary.txt
echo "Total examples: $total" >> build_summary.txt
echo "Successfully built: $success" >> build_summary.txt
echo "Failed to build: $failed" >> build_summary.txt
echo "============================================" >> build_summary.txt

# Print final summary to console
cat build_summary.txt

