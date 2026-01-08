#!/bin/bash

echo "Detailed Analysis of Other Errors"
echo "=================================="
echo

for bench in hogbom kmc kmeans memtest opticalFlow segment-reduce si tensorT tpacf xsbench convolutionDeformable daphne eikonal; do
  bench="${bench}-sycl"
  log="compilation_logs/${bench}_test.log"
  if [ -f "$log" ]; then
    echo "=== $bench ==="
    grep -E "error:|fatal" "$log" | head -3
    echo
  fi
done
