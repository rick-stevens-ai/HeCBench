#!/bin/bash

echo "Checking 'no binary' benchmarks..."
echo ""

for bench in amgmk-sycl b+tree-sycl face-sycl grep-sycl haccmk-sycl heat-sycl hybridsort-sycl srad-sycl; do
    echo "=== $bench ==="
    if [ -d "$bench" ]; then
        cd "$bench"
        # Check Makefile for program name
        if [ -f "Makefile" ]; then
            prog=$(grep "^program " Makefile | head -1 | cut -d'=' -f2 | tr -d ' ')
            echo "Makefile program: $prog"
        fi
        # List executables
        echo "Executables:"
        ls -la | grep "^-rwx" | awk '{print $9}'
        cd ..
    else
        echo "Directory not found"
    fi
    echo ""
done
