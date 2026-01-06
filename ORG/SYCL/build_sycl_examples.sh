#!/bin/bash

# Source Intel oneAPI environment
source /opt/intel/oneapi/setvars.sh --force

# Add SYCL include paths
export CPLUS_INCLUDE_PATH=/opt/intel/oneapi/compiler/2025.1/include:/opt/intel/oneapi/2025.1/include:$CPLUS_INCLUDE_PATH

# Get absolute path for logs
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LOG_DIR="${SCRIPT_DIR}/build_logs"

# Initialize log files
mkdir -p "${LOG_DIR}"
SUCCESS_LOG="${LOG_DIR}/successful_builds.log"
FAILED_LOG="${LOG_DIR}/failed_builds.log"
> "${SUCCESS_LOG}"
> "${FAILED_LOG}"

# Function to build a single example
build_example() {
    local dir="$1"
    local build_dir="${dir}/build"
    local log_file="${LOG_DIR}/$(echo ${dir} | tr './' '__').log"
    
    echo "Building $dir..."
    
    # Check if directory exists
    if [ ! -d "$dir" ]; then
        echo "✗ Directory $dir does not exist"
        echo "$dir (directory not found)" >> "${FAILED_LOG}"
        return 1
    fi
    
    cd "$dir" || {
        echo "✗ Failed to enter directory $dir"
        echo "$dir (directory access failed)" >> "${FAILED_LOG}"
        return 1
    }

    # First try CMake build
    if [ -f "CMakeLists.txt" ]; then
        mkdir -p build && cd build
        if cmake .. -DCMAKE_CXX_COMPILER=icpx \
                  -DCMAKE_C_COMPILER=icx \
                  -DCMAKE_CXX_FLAGS="-I/opt/intel/oneapi/compiler/2025.1/include -I/opt/intel/oneapi/2025.1/include" \
                  > "${log_file}" 2>&1 && \
           cmake --build . >> "${log_file}" 2>&1; then
            echo "✓ Successfully built $dir (CMake)"
            echo "$dir" >> "${SUCCESS_LOG}"
            cd "${SCRIPT_DIR}"
            return 0
        fi
        cd ..
    fi

    # If CMake failed or doesn't exist, try Make
    if [ -f "Makefile" ]; then
        # Set environment variables for Intel compilers
        export CC=icx
        export CXX=icpx
        export FC=ifx
        export CXXFLAGS="-I/opt/intel/oneapi/compiler/2025.1/include -I/opt/intel/oneapi/2025.1/include"
        
        if make clean > "${log_file}" 2>&1 && make >> "${log_file}" 2>&1; then
            echo "✓ Successfully built $dir (Make)"
            echo "$dir" >> "${SUCCESS_LOG}"
            cd "${SCRIPT_DIR}"
            return 0
        fi
    fi

    # If no build system found or build failed
    if [ ! -f "CMakeLists.txt" ] && [ ! -f "Makefile" ]; then
        echo "✗ No CMakeLists.txt or Makefile found in $dir"
        echo "$dir (no build system)" >> "${FAILED_LOG}"
    else
        echo "✗ Failed to build $dir"
        echo "$dir (build failed)" >> "${FAILED_LOG}"
        tail -n 20 "${log_file}"
    fi
    
    cd "${SCRIPT_DIR}"
    return 1
}

export -f build_example
export SUCCESS_LOG
export FAILED_LOG
export LOG_DIR
export SCRIPT_DIR

# Find all directories containing SYCL examples
echo "Finding SYCL examples..."
SYCL_DIRS=$(find . -type f \( -name "CMakeLists.txt" -o -name "Makefile" -o -name "*.cpp" -o -name "*.hpp" \) \
            -exec grep -l "SYCL\|sycl\|#include.*CL/sycl" {} \; | \
            sort -u | xargs -I {} dirname {} | sort -u)

# Count total examples
TOTAL_DIRS=$(echo "$SYCL_DIRS" | wc -l)
echo "Found $TOTAL_DIRS SYCL examples to build"
echo "SYCL directories found:"
echo "$SYCL_DIRS"
echo "------------------------"

# Build in batches of 20
echo "$SYCL_DIRS" | xargs -n1 -P 20 bash -c 'build_example "$@"' _

# Print summary
echo -e "\nBuild Summary:"
echo "Successful builds: $(wc -l < "${SUCCESS_LOG}")"
echo "Failed builds: $(wc -l < "${FAILED_LOG}")"

# List failed builds
if [ -s "${FAILED_LOG}" ]; then
    echo -e "\nFailed builds:"
    cat "${FAILED_LOG}"
fi

# List successful builds
if [ -s "${SUCCESS_LOG}" ]; then
    echo -e "\nSuccessful builds:"
    cat "${SUCCESS_LOG}"
fi
