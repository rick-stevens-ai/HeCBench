#!/bin/bash

# Comprehensive Benchmark Test Runner
# Tests all compiled SYCL benchmarks for correctness

source /opt/intel/oneapi/setvars.sh --force > /dev/null 2>&1
cd /home/stevens/HeCBench/ORG/SYCL

# Create results directories
mkdir -p test_results/logs
mkdir -p test_results/crashes

# Result counters
TOTAL=0
PASSED=0
FAILED=0
CRASHED=0
NO_DATA=0

# Result log
RESULT_FILE="test_results/benchmark_results.txt"
SUMMARY_FILE="test_results/SUMMARY.md"

echo "SYCL Benchmark Test Suite" > $RESULT_FILE
echo "=========================" >> $RESULT_FILE
echo "Start Time: $(date)" >> $RESULT_FILE
echo "" >> $RESULT_FILE

echo "Testing all compiled SYCL benchmarks..."
echo ""

# Find all benchmarks with compiled main executable
for bench in *-sycl; do
  if [ -d "$bench" ] && [ -f "$bench/main" ]; then
    ((TOTAL++))
    bench_name=$(basename "$bench")
    
    echo -n "Testing $bench_name ... "
    
    cd "$bench"
    
    # Run with timeout (30 seconds per benchmark)
    # Try with default parameters first
    timeout 30s ./main 1 > "../test_results/logs/${bench_name}.log" 2>&1
    exit_code=$?
    
    # Check results
    if [ $exit_code -eq 124 ]; then
      # Timeout
      echo "TIMEOUT"
      echo "$bench_name: TIMEOUT" >> "../$RESULT_FILE"
      ((CRASHED++))
    elif [ $exit_code -ne 0 ]; then
      # Non-zero exit (crash or error)
      echo "CRASHED (exit $exit_code)"
      echo "$bench_name: CRASHED (exit code $exit_code)" >> "../$RESULT_FILE"
      cp "../test_results/logs/${bench_name}.log" "../test_results/crashes/${bench_name}_crash.log"
      ((CRASHED++))
    else
      # Check output for PASS/FAIL
      if grep -qi "PASS" "../test_results/logs/${bench_name}.log"; then
        echo "✓ PASS"
        echo "$bench_name: PASS" >> "../$RESULT_FILE"
        ((PASSED++))
      elif grep -qi "FAIL" "../test_results/logs/${bench_name}.log"; then
        echo "✗ FAIL"
        echo "$bench_name: FAIL" >> "../$RESULT_FILE"
        ((FAILED++))
      else
        # No explicit PASS/FAIL, check for errors
        if grep -qi "error\|exception\|segmentation" "../test_results/logs/${bench_name}.log"; then
          echo "✗ ERROR"
          echo "$bench_name: ERROR (no explicit result)" >> "../$RESULT_FILE"
          ((FAILED++))
        else
          echo "? NO_DATA (completed)"
          echo "$bench_name: NO_DATA (completed successfully, no PASS/FAIL)" >> "../$RESULT_FILE"
          ((NO_DATA++))
        fi
      fi
    fi
    
    cd ..
    
    # Progress indicator every 50 benchmarks
    if [ $((TOTAL % 50)) -eq 0 ]; then
      echo ""
      echo "Progress: $TOTAL tested, $PASSED passed, $FAILED failed, $CRASHED crashed"
      echo ""
    fi
  fi
done

# Generate summary
echo "" >> $RESULT_FILE
echo "=========================" >> $RESULT_FILE
echo "Test Summary" >> $RESULT_FILE
echo "=========================" >> $RESULT_FILE
echo "Total Benchmarks: $TOTAL" >> $RESULT_FILE
echo "Passed: $PASSED" >> $RESULT_FILE
echo "Failed: $FAILED" >> $RESULT_FILE
echo "Crashed: $CRASHED" >> $RESULT_FILE
echo "No Data: $NO_DATA" >> $RESULT_FILE
echo "" >> $RESULT_FILE
echo "End Time: $(date)" >> $RESULT_FILE

# Display summary
echo ""
echo "========================================="
echo "Benchmark Test Results"
echo "========================================="
echo "Total Benchmarks Tested: $TOTAL"
echo "✓ Passed: $PASSED ($(echo "scale=1; $PASSED*100/$TOTAL" | bc)%)"
echo "✗ Failed: $FAILED ($(echo "scale=1; $FAILED*100/$TOTAL" | bc)%)"
echo "☠ Crashed: $CRASHED ($(echo "scale=1; $CRASHED*100/$TOTAL" | bc)%)"
echo "? No Data: $NO_DATA ($(echo "scale=1; $NO_DATA*100/$TOTAL" | bc)%)"
echo "========================================="
echo ""
echo "Detailed results: test_results/benchmark_results.txt"
echo "Individual logs: test_results/logs/"
echo "Crash logs: test_results/crashes/"

# Create markdown summary
cat > $SUMMARY_FILE << SUMMARY
# SYCL Benchmark Test Results

**Date:** $(date)  
**Total Benchmarks Tested:** $TOTAL / 414 expected

## Summary

| Status | Count | Percentage |
|--------|-------|------------|
| ✓ Passed | $PASSED | $(echo "scale=1; $PASSED*100/$TOTAL" | bc)% |
| ✗ Failed | $FAILED | $(echo "scale=1; $FAILED*100/$TOTAL" | bc)% |
| ☠ Crashed | $CRASHED | $(echo "scale=1; $CRASHED*100/$TOTAL" | bc)% |
| ? No Data | $NO_DATA | $(echo "scale=1; $NO_DATA*100/$TOTAL" | bc)% |

## Result Categories

### ✓ PASS ($PASSED benchmarks)
Benchmarks that explicitly output "PASS" and completed successfully.

### ✗ FAIL ($FAILED benchmarks)
Benchmarks that explicitly output "FAIL" or had runtime errors.

### ☠ CRASH ($CRASHED benchmarks)
Benchmarks that crashed, segfaulted, or timed out (>30s).

### ? NO_DATA ($NO_DATA benchmarks)
Benchmarks that completed without errors but didn't output PASS/FAIL.

## Files Generated
- \`benchmark_results.txt\` - Complete results list
- \`logs/\` - Individual benchmark output logs (all $TOTAL benchmarks)
- \`crashes/\` - Crash logs for failed benchmarks

---

**Note:** Some benchmarks may require specific input data files or GPU capabilities.
Benchmarks marked "NO_DATA" likely completed successfully but don't have explicit
validation output. Manual inspection recommended for critical benchmarks.

🤖 Generated with [Claude Code](https://claude.com/claude-code)
SUMMARY

echo "Summary report created: test_results/SUMMARY.md"
