#!/bin/bash

echo "Extracting all failed benchmarks..."

# Extract failures from batch output
grep "FAIL\|SKIP" batch_compile_output.txt | grep -v "Batch.*FAIL" | cut -d']' -f2 | cut -d'.' -f1 | sed 's/^ //g' > failed_benchmarks_list.txt

# Count by type
echo ""
echo "Failed Benchmarks Summary:"
echo "=========================="
COMPILE_FAIL=$(grep "FAIL (compilation error)" batch_compile_output.txt | wc -l)
NO_BINARY=$(grep "FAIL (no binary)" batch_compile_output.txt | wc -l)
SKIPPED=$(grep "SKIP" batch_compile_output.txt | wc -l)

echo "Compilation errors: $COMPILE_FAIL"
echo "No binary produced: $NO_BINARY"
echo "Skipped (no Makefile): $SKIPPED"
echo "TOTAL: $((COMPILE_FAIL + NO_BINARY + SKIPPED))"
echo ""

# Create categorized lists
grep "FAIL (compilation error)" batch_compile_output.txt | cut -d']' -f2 | cut -d'.' -f1 | sed 's/^ //g' > failed_compile.txt
grep "FAIL (no binary)" batch_compile_output.txt | cut -d']' -f2 | cut -d'.' -f1 | sed 's/^ //g' > failed_no_binary.txt
grep "SKIP" batch_compile_output.txt | cut -d']' -f2 | cut -d'.' -f1 | sed 's/^ //g' > failed_skipped.txt

echo "Lists created:"
echo "- failed_compile.txt (compilation errors)"
echo "- failed_no_binary.txt (no binary produced)"
echo "- failed_skipped.txt (no Makefile)"
echo "- failed_benchmarks_list.txt (all failures)"
