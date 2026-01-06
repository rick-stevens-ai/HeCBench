#!/bin/bash

# Array of benchmark directories
benchmarks=("nbody-sycl" "babelstream-sycl" "mandelbrot-sycl")

# Function to print separator
print_separator() {
    echo -e "\n=================================="
    echo "    $1"
    echo "==================================\n"
}

# Get the starting directory
base_dir=$(pwd)/..

# Loop through each benchmark
for bench in "${benchmarks[@]}"; do
    print_separator "Running $bench"
    
    cd "$base_dir/$bench"
    echo "Building $bench..."
    
    # Update Makefile if it's mandelbrot-sycl
    if [ "$bench" == "mandelbrot-sycl" ]; then
        sed -i 's/CC        = clang++/CC        = icpx/' Makefile
    fi
    
    # Clean and build
    make clean
    make
    
    echo -e "\nRunning $bench..."
    # Run with specific parameters for mandelbrot
    if [ "$bench" == "mandelbrot-sycl" ]; then
        ./main 1000
    else
        ./main
    fi
done

print_separator "All benchmarks completed"
