#!/bin/bash
for bench in *-sycl; do
  if [ -d "$bench" ] && [ ! -f "$bench/main" ]; then
    echo "$bench"
  fi
done
