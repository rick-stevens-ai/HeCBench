#!/bin/bash

# Script to help convert double precision (FP64) benchmarks to single precision (FP32)
# Usage: ./convert_fp64_to_fp32.sh <benchmark-directory>

if [ $# -ne 1 ]; then
    echo "Usage: $0 <benchmark-directory>"
    echo "Example: $0 ./my-benchmark-sycl/"
    exit 1
fi

BENCH_DIR="$1"

if [ ! -d "$BENCH_DIR" ]; then
    echo "Error: Directory not found: $BENCH_DIR"
    exit 1
fi

echo "Analyzing $BENCH_DIR for FP64 usage..."
echo ""

# Check for double type usage
echo "=== Double type declarations ==="
grep -rn "double " "$BENCH_DIR"/*.cpp "$BENCH_DIR"/*.h 2>/dev/null | head -20

echo ""
echo "=== BLAS double precision calls ==="
grep -rn "ddot\|dgemm\|dgemv\|daxpy\|dcopy" "$BENCH_DIR"/*.cpp "$BENCH_DIR"/*.h 2>/dev/null

echo ""
echo "=== Template instantiations with double ==="
grep -rn "<double>" "$BENCH_DIR"/*.cpp "$BENCH_DIR"/*.h 2>/dev/null

echo ""
echo "=== Conversion suggestions ==="
echo "1. Replace 'double' with 'float'"
echo "2. Replace 'ddot' with 'sdot'"
echo "3. Replace 'dgemm' with 'sgemm'"
echo "4. Replace 'dgemv' with 'sgemv'"
echo "5. Replace 'daxpy' with 'saxpy'"
echo "6. Replace '%lf' with '%f' in printf/scanf"
echo "7. Update precision thresholds (e.g., 1e-10 -> 1e-5)"
echo ""

# Create backup
BACKUP_DIR="${BENCH_DIR}/backup_before_fp32_conversion"
if [ ! -d "$BACKUP_DIR" ]; then
    echo "Creating backup in $BACKUP_DIR"
    mkdir -p "$BACKUP_DIR"
    cp "$BENCH_DIR"/*.cpp "$BACKUP_DIR"/ 2>/dev/null
    cp "$BENCH_DIR"/*.h "$BACKUP_DIR"/ 2>/dev/null
    echo "Backup created"
else
    echo "Backup already exists at $BACKUP_DIR"
fi

echo ""
echo "Would you like to perform automatic conversion? (y/n)"
read -r response

if [ "$response" = "y" ]; then
    echo "Performing automatic conversion..."

    for file in "$BENCH_DIR"/*.cpp "$BENCH_DIR"/*.h; do
        if [ -f "$file" ]; then
            echo "Processing: $file"

            # Create a backup
            cp "$file" "$file.bak"

            # Perform replacements (be conservative)
            # Only replace in specific contexts to avoid false positives

            # Replace double variable declarations (simple cases)
            sed -i 's/\bdouble \([a-zA-Z_][a-zA-Z0-9_]*\)/float \1/g' "$file"

            # Replace BLAS calls
            sed -i 's/\bddot\b/sdot/g' "$file"
            sed -i 's/\bdgemm\b/sgemm/g' "$file"
            sed -i 's/\bdgemv\b/sgemv/g' "$file"
            sed -i 's/\bdaxpy\b/saxpy/g' "$file"
            sed -i 's/\bdcopy\b/scopy/g' "$file"

            # Replace printf format specifiers
            sed -i 's/%lf/%f/g' "$file"

            # Replace template instantiations
            sed -i 's/<double>/<float>/g' "$file"

            echo "  Converted: $file"
            echo "  Backup: $file.bak"
        fi
    done

    echo ""
    echo "Conversion complete!"
    echo "Please review the changes and test compilation."
    echo "Original files backed up with .bak extension"
else
    echo "Automatic conversion cancelled."
    echo "You can manually edit the files or run this script again."
fi
