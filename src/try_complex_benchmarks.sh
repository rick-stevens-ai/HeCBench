#!/bin/bash

source /opt/intel/oneapi/setvars.sh --quiet 2>&1 > /dev/null

echo "Trying complex benchmarks to find easy wins..."
echo ""

# Try a selection of complex benchmarks
BENCHMARKS="bsw-sycl depixel-sycl distort-sycl sobel-sycl sad-sycl"

FIXED=0
FAILED=0

for bench in $BENCHMARKS; do
    echo "=== Trying $bench ==="
    
    if [ ! -d "$bench" ]; then
        echo "✗ Not found"
        ((FAILED++))
        continue
    fi
    
    cd "$bench"
    
    # Try quick compilation
    make CC=icpx clean > /dev/null 2>&1
    if timeout 60 make CC=icpx > /tmp/${bench}_try.log 2>&1; then
        if [ -f "main" ]; then
            echo "✓ SUCCESS - Already works!"
            ((FIXED++))
        else
            echo "→ No main binary"
        fi
    else
        # Check error type
        ERROR=$(grep "error:" /tmp/${bench}_try.log | head -1)
        echo "→ Failed: $ERROR"
        ((FAILED++))
    fi
    
    cd ..
    echo ""
done

echo "Results: Fixed=$FIXED Failed=$FAILED"
