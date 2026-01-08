#!/bin/bash

source /opt/intel/oneapi/setvars.sh --quiet 2>&1 > /dev/null

echo "Attempting more benchmark fixes..."
echo ""

# Try more complex benchmarks
BENCHMARKS="fresnel-sycl heartwall-sycl is-sycl leukocyte-sycl logan-sycl remap-sycl seam-carving-sycl slit-sycl slu-sycl sparkler-sycl sss-sycl tonemapping-sycl wlcpow-sycl wsm5-sycl xlqc-sycl"

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
    
    # Quick compile attempt
    make CC=icpx clean > /dev/null 2>&1
    timeout 60 make CC=icpx > /tmp/${bench}_compile.log 2>&1
    
    if [ $? -eq 0 ] && [ -f "main" ]; then
        echo "✓ SUCCESS!"
        ((FIXED++))
    else
        # Show first error
        ERROR=$(grep "error:" /tmp/${bench}_compile.log | head -1 | cut -c1-100)
        echo "✗ Failed: $ERROR"
        ((FAILED++))
    fi
    
    cd ..
    echo ""
done

echo "Results: Fixed=$FIXED Failed=$FAILED"
