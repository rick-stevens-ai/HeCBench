#!/bin/bash

# Source Intel oneAPI environment
source /opt/intel/oneapi/setvars.sh --force

# Initialize log files
mkdir -p build_logs
SUCCESS_LOG="build_logs/successful_builds.log"
FAILED_LOG="build_logs/failed_builds.log"
> $SUCCESS_LOG
> $FAILED_LOG

# Find all directories containing SYCL examples
SYCL_DIRS=$(find . -type f -name "CMakeLists.txt" -exec grep -l "SYCL" {} \; | xargs -I {} dirname {})

# Count total examples
TOTAL_DIRS=$(echo "$SYCL_DIRS" | wc -l)
echo "Found $TOTAL_DIRS SYCL examples to build"

# Function to build a single example
build_example() {
    local dir=$1
    local build_dir="${dir}/build"
    local log_file="build_logs/$(echo ${dir} | tr '/' '_').log"
    
    echo "Building $dir..."
    mkdir -p "$build_dir"
    cd "$build_dir"
    
    # Attempt to build
    if cmake .. > "$log_file" 2>&1 && make >> "$log_file" 2>&1; then
        echo "$dir" >> $SUCCESS_LOG
        echo "✓ Successfully built $dir"
    else
        echo "$dir" >> $FAILED_LOG
        echo "✗ Failed to build $dir"
    fi
    
    cd - > /dev/null
}

export -f build_example
export SUCCESS_LOG
export FAILED_LOG

# Build in batches of 20
echo "$SYCL_DIRS" | xargs -I {} -P 20 bash -c 'build_example "{}"'

# Print summary
echo "Build Summary:"
echo "Successful builds: $(wc -l < $SUCCESS_LOG)"
echo "Failed builds: $(wc -l < $FAILED_LOG)"

# List failed builds
if [ -s "$FAILED_LOG" ]; then
    echo -e "\nFailed builds:"
    cat "$FAILED_LOG"
fi
