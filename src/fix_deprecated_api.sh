#!/bin/bash

source /opt/intel/oneapi/setvars.sh --quiet 2>&1 > /dev/null

echo "Fixing deprecated SYCL API (local accessor)..."
echo ""

# Deprecated API benchmarks (excluding axhelm which is done)
BENCHMARKS="convolutionDeformable-sycl frechet-sycl rsmt-sycl shuffle-sycl stsg-sycl tensorT-sycl"

FIXED=0
FAILED=0

for bench in $BENCHMARKS; do
    echo "=== $bench ==="
    
    if [ ! -d "$bench" ]; then
        echo "✗ Not found"
        ((FAILED++))
        continue
    fi
    
    cd "$bench"
    
    # Show current error
    tail -3 ../compilation_logs_20260108_102500/${bench}.log 2>/dev/null | grep "error:"
    
    echo "Attempting fix..."
    
    # Try compilation to see if it's really just deprecated API
    make CC=icpx clean > /dev/null 2>&1
    timeout 60 make CC=icpx > /tmp/${bench}_attempt.log 2>&1
    
    # Check error type
    if grep -q "deprecated.*local_accessor\|access::target::local" /tmp/${bench}_attempt.log; then
        echo "→ Deprecated API confirmed"
        # This needs manual fixing - flag for review
        echo "$bench" >> ../needs_manual_local_accessor_fix.txt
    else
        ERROR=$(grep "error:" /tmp/${bench}_attempt.log | head -1)
        echo "→ Different error: $ERROR"
    fi
    
    cd ..
    echo ""
done

echo "Summary:"
echo "--------"
if [ -f "needs_manual_local_accessor_fix.txt" ]; then
    echo "Benchmarks needing local_accessor updates:"
    cat needs_manual_local_accessor_fix.txt
fi
