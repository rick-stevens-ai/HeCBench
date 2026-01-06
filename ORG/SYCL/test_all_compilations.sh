#!/bin/bash

# Comprehensive compilation test script for SYCL benchmarks
# Tests all benchmarks and categorizes errors

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
LOG_DIR="$SCRIPT_DIR/compilation_logs"
RESULTS_FILE="$SCRIPT_DIR/compilation_summary.txt"
DETAILED_RESULTS="$SCRIPT_DIR/compilation_detailed.txt"
ERROR_CATEGORIES="$SCRIPT_DIR/error_categories.txt"

# Create log directory
mkdir -p "$LOG_DIR"

# Clear results files
> "$RESULTS_FILE"
> "$DETAILED_RESULTS"
> "$ERROR_CATEGORIES"

# Source Intel OneAPI environment
echo "Sourcing Intel OneAPI environment..."
source /opt/intel/oneapi/setvars.sh --force > /dev/null 2>&1

# Counters
TOTAL=0
ALREADY_COMPILED=0
COMPILED_NOW=0
FAIL_FP64=0
FAIL_MISSING_FILE=0
FAIL_INCLUDE_PATH=0
FAIL_COMPILE_ERROR=0
FAIL_LINKER_ERROR=0
FAIL_UNKNOWN=0

echo "========================================="
echo "Starting compilation test of all SYCL benchmarks"
echo "========================================="
echo ""

# Get list of all SYCL benchmarks
cd "$SCRIPT_DIR"
for bench_dir in *-sycl; do
    if [ ! -d "$bench_dir" ]; then
        continue
    fi

    TOTAL=$((TOTAL + 1))
    bench=$(basename "$bench_dir")
    log_file="$LOG_DIR/${bench}.log"

    # Progress indicator
    if [ $((TOTAL % 20)) -eq 0 ]; then
        echo "Processed $TOTAL benchmarks..."
    fi

    cd "$SCRIPT_DIR/$bench_dir" || continue

    # Check if already compiled
    if [ -f "main" ]; then
        ALREADY_COMPILED=$((ALREADY_COMPILED + 1))
        echo "$bench: ALREADY_COMPILED" >> "$RESULTS_FILE"
        echo "✓ $bench: Already compiled" >> "$DETAILED_RESULTS"
        continue
    fi

    # Check if Makefile exists
    if [ ! -f "Makefile" ]; then
        echo "$bench: NO_MAKEFILE" >> "$RESULTS_FILE"
        echo "✗ $bench: No Makefile found" >> "$DETAILED_RESULTS"
        echo "NO_MAKEFILE: $bench" >> "$ERROR_CATEGORIES"
        FAIL_UNKNOWN=$((FAIL_UNKNOWN + 1))
        continue
    fi

    # Try to compile - first clean
    make clean > /dev/null 2>&1

    # Try with icpx
    timeout 60s make CC=icpx > "$log_file" 2>&1
    compile_result=$?

    if [ $compile_result -eq 0 ] && [ -f "main" ]; then
        COMPILED_NOW=$((COMPILED_NOW + 1))
        echo "$bench: COMPILED_SUCCESS" >> "$RESULTS_FILE"
        echo "✓ $bench: Compiled successfully" >> "$DETAILED_RESULTS"
    else
        # Analyze the error
        if grep -q "Double type is not supported\|FP64\|ddot\|dgemm" "$log_file"; then
            FAIL_FP64=$((FAIL_FP64 + 1))
            echo "$bench: FAIL_FP64" >> "$RESULTS_FILE"
            echo "✗ $bench: FP64 not supported" >> "$DETAILED_RESULTS"
            echo "FP64: $bench" >> "$ERROR_CATEGORIES"

        elif grep -q "No such file or directory.*\.h\|cannot find.*\.h" "$log_file"; then
            # Check if it's an include path issue
            if grep -q "\.\./.*-cuda" "$log_file"; then
                FAIL_INCLUDE_PATH=$((FAIL_INCLUDE_PATH + 1))
                echo "$bench: FAIL_INCLUDE_PATH" >> "$RESULTS_FILE"
                echo "✗ $bench: Missing include path (needs CUDA reference)" >> "$DETAILED_RESULTS"
                echo "INCLUDE_PATH: $bench" >> "$ERROR_CATEGORIES"
            else
                FAIL_MISSING_FILE=$((FAIL_MISSING_FILE + 1))
                echo "$bench: FAIL_MISSING_FILE" >> "$RESULTS_FILE"
                echo "✗ $bench: Missing file" >> "$DETAILED_RESULTS"
                echo "MISSING_FILE: $bench" >> "$ERROR_CATEGORIES"
            fi

        elif grep -q "undefined reference\|cannot find -l" "$log_file"; then
            FAIL_LINKER_ERROR=$((FAIL_LINKER_ERROR + 1))
            echo "$bench: FAIL_LINKER" >> "$RESULTS_FILE"
            echo "✗ $bench: Linker error" >> "$DETAILED_RESULTS"
            echo "LINKER: $bench" >> "$ERROR_CATEGORIES"

        elif grep -q "error:" "$log_file"; then
            FAIL_COMPILE_ERROR=$((FAIL_COMPILE_ERROR + 1))
            echo "$bench: FAIL_COMPILE" >> "$RESULTS_FILE"
            # Extract first error line for debugging
            first_error=$(grep "error:" "$log_file" | head -1)
            echo "✗ $bench: Compile error - $first_error" >> "$DETAILED_RESULTS"
            echo "COMPILE_ERROR: $bench" >> "$ERROR_CATEGORIES"

        elif [ $compile_result -eq 124 ]; then
            echo "$bench: TIMEOUT" >> "$RESULTS_FILE"
            echo "✗ $bench: Compilation timeout" >> "$DETAILED_RESULTS"
            echo "TIMEOUT: $bench" >> "$ERROR_CATEGORIES"
            FAIL_UNKNOWN=$((FAIL_UNKNOWN + 1))

        else
            FAIL_UNKNOWN=$((FAIL_UNKNOWN + 1))
            echo "$bench: FAIL_UNKNOWN" >> "$RESULTS_FILE"
            echo "✗ $bench: Unknown error" >> "$DETAILED_RESULTS"
            echo "UNKNOWN: $bench" >> "$ERROR_CATEGORIES"
        fi
    fi
done

cd "$SCRIPT_DIR"

# Generate summary report
echo ""
echo "========================================="
echo "COMPILATION TEST SUMMARY"
echo "========================================="
echo ""
echo "Total benchmarks tested:    $TOTAL"
echo "Already compiled:           $ALREADY_COMPILED"
echo "Newly compiled:             $COMPILED_NOW"
echo "----------------------------------------"
echo "TOTAL WORKING:              $((ALREADY_COMPILED + COMPILED_NOW))"
echo "----------------------------------------"
echo ""
echo "FAILURES BY CATEGORY:"
echo "  FP64 not supported:       $FAIL_FP64"
echo "  Include path issues:      $FAIL_INCLUDE_PATH"
echo "  Missing files:            $FAIL_MISSING_FILE"
echo "  Compile errors:           $FAIL_COMPILE_ERROR"
echo "  Linker errors:            $FAIL_LINKER_ERROR"
echo "  Unknown/Other:            $FAIL_UNKNOWN"
echo "----------------------------------------"
echo "TOTAL FAILURES:             $((FAIL_FP64 + FAIL_INCLUDE_PATH + FAIL_MISSING_FILE + FAIL_COMPILE_ERROR + FAIL_LINKER_ERROR + FAIL_UNKNOWN))"
echo ""
echo "Results saved to:"
echo "  Summary:     $RESULTS_FILE"
echo "  Detailed:    $DETAILED_RESULTS"
echo "  Categories:  $ERROR_CATEGORIES"
echo "  Logs:        $LOG_DIR/"
echo ""

# Save summary to file
cat > "$SCRIPT_DIR/COMPILATION_SUMMARY.md" <<EOF
# SYCL Benchmark Compilation Summary

**Test Date:** $(date)
**Total Benchmarks:** $TOTAL

## Results Overview

| Status | Count | Percentage |
|--------|-------|------------|
| Already Compiled | $ALREADY_COMPILED | $(awk "BEGIN {printf \"%.1f\", ($ALREADY_COMPILED/$TOTAL)*100}")% |
| Newly Compiled | $COMPILED_NOW | $(awk "BEGIN {printf \"%.1f\", ($COMPILED_NOW/$TOTAL)*100}")% |
| **TOTAL WORKING** | **$((ALREADY_COMPILED + COMPILED_NOW))** | **$(awk "BEGIN {printf \"%.1f\", (($ALREADY_COMPILED + $COMPILED_NOW)/$TOTAL)*100}")**% |

## Failures by Category

| Error Type | Count | Priority |
|------------|-------|----------|
| FP64 not supported | $FAIL_FP64 | HIGH - Batch fix possible |
| Include path issues | $FAIL_INCLUDE_PATH | MEDIUM - Need CUDA references |
| Compile errors | $FAIL_COMPILE_ERROR | MEDIUM - Need investigation |
| Missing files | $FAIL_MISSING_FILE | LOW - May need external data |
| Linker errors | $FAIL_LINKER_ERROR | MEDIUM - Library issues |
| Unknown/Other | $FAIL_UNKNOWN | LOW - Need manual review |

## Prioritized Action Plan

### Priority 1: FP64 Issues ($FAIL_FP64 benchmarks)
These can be batch-fixed by converting double to float.
Use: \`./convert_fp64_to_fp32.sh <benchmark>\`

### Priority 2: Include Path Issues ($FAIL_INCLUDE_PATH benchmarks)
Need to fix Makefile include paths to point to \`../../CUDA/\` instead of \`../\`

### Priority 3: Compile Errors ($FAIL_COMPILE_ERROR benchmarks)
Need individual investigation - check logs in \`$LOG_DIR/\`

### Priority 4: Other Issues
Linker errors, missing files, and unknown issues need manual review.

## Files Generated

- \`$RESULTS_FILE\` - One-line status per benchmark
- \`$DETAILED_RESULTS\` - Detailed results with error descriptions
- \`$ERROR_CATEGORIES\` - Benchmarks grouped by error type
- \`$LOG_DIR/\` - Full compilation logs for each benchmark

EOF

echo "Detailed report saved to: $SCRIPT_DIR/COMPILATION_SUMMARY.md"
