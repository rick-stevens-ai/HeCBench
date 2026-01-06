#!/bin/bash

# Comprehensive compilation test script for SYCL benchmarks - Version 2
# More robust version with better directory handling

SCRIPT_DIR="/home/stevens/HeCBench/ORG/SYCL"
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
FAIL_NO_MAKEFILE=0
FAIL_TIMEOUT=0
FAIL_UNKNOWN=0

echo "========================================="
echo "Starting compilation test of all SYCL benchmarks"
echo "========================================="
echo ""

# Get list of all SYCL benchmarks
cd "$SCRIPT_DIR" || exit 1

for bench in *-sycl; do
    # Make sure it's a directory
    if [ ! -d "$bench" ]; then
        continue
    fi

    TOTAL=$((TOTAL + 1))
    log_file="$LOG_DIR/${bench}.log"

    # Progress indicator
    if [ $((TOTAL % 50)) -eq 0 ]; then
        echo "Processed $TOTAL benchmarks..."
    fi

    # Check if already compiled
    if [ -f "$bench/main" ]; then
        ALREADY_COMPILED=$((ALREADY_COMPILED + 1))
        echo "$bench: ALREADY_COMPILED" >> "$RESULTS_FILE"
        continue
    fi

    # Check if Makefile exists
    if [ ! -f "$bench/Makefile" ]; then
        echo "$bench: NO_MAKEFILE" >> "$RESULTS_FILE"
        echo "NO_MAKEFILE: $bench" >> "$ERROR_CATEGORIES"
        FAIL_NO_MAKEFILE=$((FAIL_NO_MAKEFILE + 1))
        continue
    fi

    # Try to compile in subshell to avoid directory issues
    (
        cd "$bench" || exit 1
        make clean > /dev/null 2>&1
        timeout 60s make CC=icpx > "$log_file" 2>&1
    )
    compile_result=$?

    # Check if compilation succeeded
    if [ $compile_result -eq 0 ] && [ -f "$bench/main" ]; then
        COMPILED_NOW=$((COMPILED_NOW + 1))
        echo "$bench: COMPILED_SUCCESS" >> "$RESULTS_FILE"

    elif [ $compile_result -eq 124 ]; then
        # Timeout
        echo "$bench: TIMEOUT" >> "$RESULTS_FILE"
        echo "TIMEOUT: $bench" >> "$ERROR_CATEGORIES"
        FAIL_TIMEOUT=$((FAIL_TIMEOUT + 1))

    else
        # Analyze the error
        if grep -qi "Double type is not supported\|FP64\|ddot\|dgemm\|daxpy\|dcopy" "$log_file"; then
            FAIL_FP64=$((FAIL_FP64 + 1))
            echo "$bench: FAIL_FP64" >> "$RESULTS_FILE"
            echo "FP64: $bench" >> "$ERROR_CATEGORIES"

        elif grep -qi "No such file or directory.*\.h\|cannot find.*\.h" "$log_file"; then
            # Check if it's an include path issue
            if grep -q "\.\./.*-cuda\|atomicIntrinsics-cuda" "$log_file" 2>/dev/null; then
                FAIL_INCLUDE_PATH=$((FAIL_INCLUDE_PATH + 1))
                echo "$bench: FAIL_INCLUDE_PATH" >> "$RESULTS_FILE"
                echo "INCLUDE_PATH: $bench" >> "$ERROR_CATEGORIES"
            else
                FAIL_MISSING_FILE=$((FAIL_MISSING_FILE + 1))
                echo "$bench: FAIL_MISSING_FILE" >> "$RESULTS_FILE"
                echo "MISSING_FILE: $bench" >> "$ERROR_CATEGORIES"
            fi

        elif grep -qi "undefined reference\|cannot find -l" "$log_file"; then
            FAIL_LINKER_ERROR=$((FAIL_LINKER_ERROR + 1))
            echo "$bench: FAIL_LINKER" >> "$RESULTS_FILE"
            echo "LINKER: $bench" >> "$ERROR_CATEGORIES"

        elif grep -qi "error:" "$log_file"; then
            FAIL_COMPILE_ERROR=$((FAIL_COMPILE_ERROR + 1))
            echo "$bench: FAIL_COMPILE" >> "$RESULTS_FILE"
            echo "COMPILE_ERROR: $bench" >> "$ERROR_CATEGORIES"

        else
            FAIL_UNKNOWN=$((FAIL_UNKNOWN + 1))
            echo "$bench: FAIL_UNKNOWN" >> "$RESULTS_FILE"
            echo "UNKNOWN: $bench" >> "$ERROR_CATEGORIES"
        fi
    fi
done

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
echo "  No Makefile:              $FAIL_NO_MAKEFILE"
echo "  Timeout:                  $FAIL_TIMEOUT"
echo "  Unknown/Other:            $FAIL_UNKNOWN"
echo "----------------------------------------"
TOTAL_FAILURES=$((FAIL_FP64 + FAIL_INCLUDE_PATH + FAIL_MISSING_FILE + FAIL_COMPILE_ERROR + FAIL_LINKER_ERROR + FAIL_NO_MAKEFILE + FAIL_TIMEOUT + FAIL_UNKNOWN))
echo "TOTAL FAILURES:             $TOTAL_FAILURES"
echo ""

# Create detailed categorized lists
echo "Creating categorized error lists..."
grep "^FP64:" "$ERROR_CATEGORIES" > "$SCRIPT_DIR/errors_fp64.txt" 2>/dev/null
grep "^INCLUDE_PATH:" "$ERROR_CATEGORIES" > "$SCRIPT_DIR/errors_include_path.txt" 2>/dev/null
grep "^COMPILE_ERROR:" "$ERROR_CATEGORIES" > "$SCRIPT_DIR/errors_compile.txt" 2>/dev/null
grep "^LINKER:" "$ERROR_CATEGORIES" > "$SCRIPT_DIR/errors_linker.txt" 2>/dev/null

echo "Results saved to:"
echo "  Summary:     $RESULTS_FILE"
echo "  Categories:  $ERROR_CATEGORIES"
echo "  FP64 list:   errors_fp64.txt"
echo "  Include:     errors_include_path.txt"
echo "  Compile:     errors_compile.txt"
echo "  Linker:      errors_linker.txt"
echo "  Logs:        $LOG_DIR/"
echo ""

# Save summary markdown
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

| Error Type | Count | Priority | Fix Strategy |
|------------|-------|----------|--------------|
| FP64 not supported | $FAIL_FP64 | **HIGH** | Batch fix with convert_fp64_to_fp32.sh |
| Include path issues | $FAIL_INCLUDE_PATH | MEDIUM | Fix Makefile paths to ../../CUDA/ |
| Compile errors | $FAIL_COMPILE_ERROR | MEDIUM | Individual investigation needed |
| Linker errors | $FAIL_LINKER_ERROR | MEDIUM | Library/dependency issues |
| Missing files | $FAIL_MISSING_FILE | LOW | May need external data files |
| No Makefile | $FAIL_NO_MAKEFILE | LOW | Incomplete benchmark |
| Timeout | $FAIL_TIMEOUT | LOW | Very slow compilation |
| Unknown/Other | $FAIL_UNKNOWN | LOW | Manual review needed |

## Prioritized Action Plan

### 🔴 Priority 1: FP64 Issues ($FAIL_FP64 benchmarks)
These can be batch-fixed by converting double→float. See \`errors_fp64.txt\`

**Action:**
\`\`\`bash
# For each benchmark in errors_fp64.txt:
./convert_fp64_to_fp32.sh <benchmark-directory>
\`\`\`

### 🟡 Priority 2: Include Path Issues ($FAIL_INCLUDE_PATH benchmarks)
Fix Makefile include paths. See \`errors_include_path.txt\`

**Action:** Update Makefiles to use \`../../CUDA/\` instead of \`../\`

### 🟡 Priority 3: Compile Errors ($FAIL_COMPILE_ERROR benchmarks)
Need individual investigation. See \`errors_compile.txt\` and logs in \`compilation_logs/\`

### 🔵 Priority 4: Other Issues
Linker errors, missing files, and unknown issues need case-by-case review.

## Files Generated

- \`compilation_summary.txt\` - One-line status per benchmark
- \`error_categories.txt\` - All errors with categories
- \`errors_fp64.txt\` - List of FP64 issues only
- \`errors_include_path.txt\` - List of include path issues
- \`errors_compile.txt\` - List of compile errors
- \`errors_linker.txt\` - List of linker errors
- \`compilation_logs/\` - Full compilation log for each benchmark

EOF

echo "Detailed report saved to: COMPILATION_SUMMARY.md"
echo ""
echo "Quick stats:"
echo "  Working: $((ALREADY_COMPILED + COMPILED_NOW))/$TOTAL ($(awk "BEGIN {printf \"%.1f\", (($ALREADY_COMPILED + $COMPILED_NOW)/$TOTAL)*100}")%)"
echo "  Failed:  $TOTAL_FAILURES/$TOTAL ($(awk "BEGIN {printf \"%.1f\", ($TOTAL_FAILURES/$TOTAL)*100}")%)"
