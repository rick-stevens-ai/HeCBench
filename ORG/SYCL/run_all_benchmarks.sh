#!/bin/bash

# Handle interrupts
trap 'echo -e "\n\nScript interrupted. Progress saved."; exit 1' INT TERM

# Initialize or read state
STATE_FILE="benchmark_state.txt"
if [ -f "$STATE_FILE" ]; then
    COMPLETED=($(cat "$STATE_FILE"))
else
    COMPLETED=()
fi

# Initialize files
> working.txt
> failed.txt
> benchmark_summary.txt
> build_errors.log

# Initialize counters
success_count=0
fail_count=0

# Get total number of benchmarks
total=$(wc -l < sycl_benchmarks.txt)
current=0

# Function to print progress
print_progress() {
    current=$1
    percentage=$(( (current * 100) / total ))
    printf "\rProgress: %d/%d (%d%%) - Success: %d, Failed: %d - %s" $current $total $percentage $success_count $fail_count "$(date '+%H:%M:%S')"
}

# Function to log summary
log_summary() {
    echo "[$(date '+%Y-%m-%d %H:%M:%S')] $1: $2 - $3" >> benchmark_summary.txt
}

# Read benchmarks line by line
while IFS= read -r benchmark; do
    current=$((current + 1))

    # Skip if already completed
    if [[ " ${COMPLETED[@]} " =~ " ${benchmark} " ]]; then
        print_progress $current
        continue
    fi

    print_progress $current
    echo -e "\n\nTesting: $benchmark"
    
    # Change to benchmark directory
    cd "$benchmark" 2>/dev/null
    if [ $? -ne 0 ]; then
        echo "$benchmark" >> ../failed.txt
        log_summary "$benchmark" "FAILED" "Directory access failed"
        echo "Failed to enter directory"
        ((fail_count++))
        cd ..
        continue
    fi

    # Update Makefile if necessary
    if [ -f Makefile ]; then
        sed -i 's/CC.*=.*clang++/CC        = icpx/' Makefile 2>/dev/null
    fi

    # Try to build
    make clean >/dev/null 2>&1
    echo "=== Build Output for $benchmark ===" >> ../build_errors.log
    if ! make 2>> ../build_errors.log; then
        echo "$benchmark" >> ../failed.txt
        log_summary "$benchmark" "FAILED" "Build failed (see build_errors.log)"
        echo "Build failed"
        ((fail_count++))
        cd ..
        COMPLETED+=("$benchmark")
        echo "${COMPLETED[@]}" > "../$STATE_FILE"
        continue
    fi

    # Try to run
    timeout 30s ./main >../build_errors.log 2>&1
    run_status=$?
    
    if [ $run_status -eq 0 ]; then
        echo "$benchmark" >> ../working.txt
        log_summary "$benchmark" "SUCCESS" "Build and run successful"
        echo "Success"
        ((success_count++))
    else
        echo "$benchmark" >> ../failed.txt
        if [ $run_status -eq 124 ]; then
            log_summary "$benchmark" "FAILED" "Timeout after 30s"
            echo "Run timed out"
        else
            log_summary "$benchmark" "FAILED" "Run failed with status $run_status"
            echo "Run failed"
        fi
        ((fail_count++))
    fi

    cd ..
    COMPLETED+=("$benchmark")
    echo "${COMPLETED[@]}" > "$STATE_FILE"
done < sycl_benchmarks.txt

echo -e "\n\nTesting completed!"
echo "Working benchmarks: $success_count"
echo "Failed benchmarks: $fail_count"
echo "Success rate: $(( (success_count * 100) / (success_count + fail_count) ))%"
echo "Detailed summary available in benchmark_summary.txt"
echo "Build errors available in build_errors.log"

# Clean up state file if completed successfully
rm -f "$STATE_FILE"
