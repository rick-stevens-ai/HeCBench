#!/bin/bash

source /opt/intel/oneapi/setvars.sh --quiet 2>&1 > /dev/null

echo "Fixing namespace issues (std::fabs -> sycl::fabs)..."
echo ""

# List of benchmarks with namespace issues
BENCHMARKS="btree-sycl gibbs-sycl rushlarsen-sycl"

FIXED=0
FAILED=0

for bench in $BENCHMARKS; do
    echo -n "Fixing $bench... "
    
    if [ ! -d "$bench" ]; then
        echo "✗ Not found"
        ((FAILED++))
        continue
    fi
    
    cd "$bench"
    
    # Find and fix std::fabs -> sycl::fabs
    if grep -q "std::fabs" *.cpp *.h 2>/dev/null; then
        sed -i 's/std::fabs/sycl::fabs/g' *.cpp *.h 2>/dev/null
        echo -n "patched... "
    fi
    
    # Try compilation
    make CC=icpx clean > /dev/null 2>&1
    if timeout 60 make CC=icpx > /tmp/${bench}_fix.log 2>&1; then
        if [ -f "main" ] || [ -L "main" ]; then
            echo "✓ FIXED"
            ((FIXED++))
        else
            # Check for alternative binary name
            BINARY=$(grep "^program " Makefile 2>/dev/null | cut -d'=' -f2 | tr -d ' ')
            if [ -n "$BINARY" ] && [ -f "$BINARY" ]; then
                ln -sf "$BINARY" main
                echo "✓ FIXED (linked $BINARY)"
                ((FIXED++))
            else
                echo "✗ No binary"
                ((FAILED++))
            fi
        fi
    else
        echo "✗ Compilation failed"
        tail -5 /tmp/${bench}_fix.log
        ((FAILED++))
    fi
    
    cd ..
done

echo ""
echo "Fixed: $FIXED / Failed: $FAILED"
