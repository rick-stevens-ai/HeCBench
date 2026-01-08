#!/bin/bash
source /opt/intel/oneapi/setvars.sh --force > /dev/null 2>&1

echo "Testing Remaining Makefile Path Issues"
echo "======================================="

# opticalFlow - Makefile syntax error
echo -n "opticalFlow-sycl: "
cd opticalFlow-sycl
if timeout 30s make > ../compilation_logs/opticalFlow_retest.log 2>&1; then
  echo "✓ WORKS"
else
  echo "✗ FAILS - $(grep -i 'error:' ../compilation_logs/opticalFlow_retest.log | head -1 | cut -c1-60)"
fi
cd ..

# memtest - nd_item issue
echo -n "memtest-sycl: "
cd memtest-sycl
if timeout 30s make > ../compilation_logs/memtest_retest.log 2>&1; then
  echo "✓ WORKS"
else
  echo "✗ FAILS - $(grep -i 'error:' ../compilation_logs/memtest_retest.log | head -1 | cut -c1-60)"
fi
cd ..

# kmeans - no Makefile
echo -n "kmeans-sycl: "
if [ ! -f "kmeans-sycl/Makefile" ]; then
  echo "✗ No Makefile"
else
  echo "Has Makefile now"
fi

