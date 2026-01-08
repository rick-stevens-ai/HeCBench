#!/bin/bash

echo "Fixing 'no binary' issues by creating 'main' symlinks..."
echo ""

# Map of benchmark -> actual executable
declare -A executables=(
    ["amgmk-sycl"]="AMGMk"
    ["b+tree-sycl"]="b+tree.out"
    ["face-sycl"]="vj-gpu"  # Use GPU version
    ["grep-sycl"]="nfa"
    ["haccmk-sycl"]="haccmk"
    ["heat-sycl"]="heat"
    ["hybridsort-sycl"]="hybridsort"
    ["srad-sycl"]="srad"
)

FIXED=0
for bench in "${!executables[@]}"; do
    exe="${executables[$bench]}"
    echo -n "Fixing $bench ($exe)... "
    
    if [ -d "$bench" ] && [ -f "$bench/$exe" ]; then
        cd "$bench"
        ln -sf "$exe" main
        echo "✓ Created symlink"
        ((FIXED++))
        cd ..
    else
        echo "✗ Not found"
    fi
done

echo ""
echo "Fixed: $FIXED/8"
