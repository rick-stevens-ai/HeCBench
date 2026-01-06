#!/bin/bash
set -e
echo "Running all benchmarks for Intel GPU..."
for dir in */ ; do
    if [ -f "$dir/Makefile" ] || [ -f "$dir/makefile" ]; then
        echo "--------------------------------------------"
        echo "Entering benchmark directory: $dir"
        (cd "$dir" && make CC=icpx run)
        echo "Finished benchmark in $dir"
    else
        echo "Skipping directory $dir (no Makefile found)"
    fi
done
echo "--------------------------------------------"
echo "All benchmarks processed."
