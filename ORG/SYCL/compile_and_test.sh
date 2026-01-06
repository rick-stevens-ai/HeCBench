#!/bin/bash

# Script to systematically compile and test SYCL benchmarks
# Usage: ./compile_and_test.sh [benchmark-name]
#        ./compile_and_test.sh --all    # Test all benchmarks

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
LOG_DIR="$SCRIPT_DIR/test_logs"
RESULTS_FILE="$SCRIPT_DIR/compilation_results.txt"

# Create log directory
mkdir -p "$LOG_DIR"

# Source Intel OneAPI environment
source /opt/intel/oneapi/setvars.sh --force > /dev/null 2>&1

# Function to test a single benchmark
test_benchmark() {
    local bench="$1"
    local bench_dir="$SCRIPT_DIR/$bench"

    if [ ! -d "$bench_dir" ]; then
        echo "Directory not found: $bench_dir"
        return 1
    fi

    echo "========================================="
    echo "Testing: $bench"
    echo "========================================="

    cd "$bench_dir" || return 1

    local log_file="$LOG_DIR/${bench}.log"

    # Check if already compiled
    if [ -f "main" ]; then
        echo "  [INFO] Executable already exists"

        # Try to run with --help
        if ./main --help > "$log_file" 2>&1; then
            echo "  [PASS] Executable runs with --help"
            echo "$bench: COMPILED, HELP_OK" >> "$RESULTS_FILE"
        elif ./main > /dev/null 2>&1; then
            echo "  [PASS] Executable runs"
            echo "$bench: COMPILED, RUNS" >> "$RESULTS_FILE"
        else
            echo "  [WARN] Executable exists but may need arguments"
            echo "$bench: COMPILED, NEEDS_ARGS" >> "$RESULTS_FILE"
        fi
        return 0
    fi

    # Try to compile
    echo "  [INFO] Attempting compilation..."

    # Try with icpx
    if make CC=icpx > "$log_file" 2>&1; then
        echo "  [PASS] Compilation successful with icpx"
        echo "$bench: COMPILED_NEW" >> "$RESULTS_FILE"

        # Try to run
        if [ -f "main" ]; then
            if ./main --help > /dev/null 2>&1; then
                echo "  [INFO] Runs with --help"
            elif ./main > /dev/null 2>&1; then
                echo "  [INFO] Runs without args"
            else
                echo "  [INFO] May need specific arguments"
            fi
        fi
    else
        # Check for specific errors
        if grep -q "Double type is not supported" "$log_file"; then
            echo "  [FAIL] FP64 not supported"
            echo "$bench: FAIL_FP64" >> "$RESULTS_FILE"
        elif grep -q "No such file or directory" "$log_file"; then
            echo "  [FAIL] Missing include or source file"
            echo "$bench: FAIL_MISSING_FILE" >> "$RESULTS_FILE"
        elif grep -q "error:" "$log_file"; then
            echo "  [FAIL] Compilation error (see log)"
            echo "$bench: FAIL_COMPILE" >> "$RESULTS_FILE"
        else
            echo "  [FAIL] Unknown error"
            echo "$bench: FAIL_UNKNOWN" >> "$RESULTS_FILE"
        fi
    fi

    cd "$SCRIPT_DIR"
}

# Main script
if [ "$1" == "--all" ]; then
    # Clear results file
    > "$RESULTS_FILE"

    echo "Testing all SYCL benchmarks..."
    for bench_dir in *-sycl/; do
        bench=$(basename "$bench_dir")
        test_benchmark "$bench"
    done

    echo ""
    echo "========================================="
    echo "Summary"
    echo "========================================="
    echo "Total tested: $(wc -l < "$RESULTS_FILE")"
    echo "Compiled: $(grep -c "COMPILED" "$RESULTS_FILE")"
    echo "FP64 issues: $(grep -c "FAIL_FP64" "$RESULTS_FILE")"
    echo "Missing files: $(grep -c "FAIL_MISSING_FILE" "$RESULTS_FILE")"
    echo "Compile errors: $(grep -c "FAIL_COMPILE" "$RESULTS_FILE")"

elif [ -n "$1" ]; then
    test_benchmark "$1"
else
    echo "Usage: $0 <benchmark-name>"
    echo "       $0 --all"
    echo ""
    echo "Examples:"
    echo "  $0 babelstream-sycl"
    echo "  $0 --all"
fi
