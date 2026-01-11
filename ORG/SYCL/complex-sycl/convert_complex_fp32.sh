#!/bin/bash
cd /home/stevens/HeCBench/ORG/SYCL/complex-sycl

for file in main.cpp complex.h kernels.h reference.h; do
  echo "Converting $file..."
  # Replace double with float
  sed -i 's/\bdouble\b/float/g' "$file"
  # Add 'f' suffix to float literals (but be careful not to double-add)
  sed -i 's/\b\([0-9]\+\.[0-9]\+\)\([^f0-9]\)/\1f\2/g' "$file"
  sed -i 's/\b\([0-9]\+\.[0-9]\+\)$/\1f/g' "$file"
done

echo "Conversion complete"
