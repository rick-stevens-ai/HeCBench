#!/bin/bash

# Script to analyze compilation error patterns

SCRIPT_DIR="/home/stevens/HeCBench/ORG/SYCL"
LOG_DIR="$SCRIPT_DIR/compilation_logs"
OUTPUT="$SCRIPT_DIR/ERROR_ANALYSIS.md"

cd "$SCRIPT_DIR" || exit 1

echo "Analyzing compilation errors..."

# Create categorized lists
> errors_wrong_compiler.txt
> errors_missing_headers.txt
> errors_deprecated_api.txt
> errors_shuffle_functions.txt
> errors_intel_extensions.txt
> errors_other.txt

# Analyze each failed benchmark
for bench in $(grep "FAIL_COMPILE\|FAIL_UNKNOWN\|FAIL_LINKER" compilation_summary.txt | cut -d: -f1); do
    log_file="$LOG_DIR/${bench}.log"

    if [ ! -f "$log_file" ]; then
        continue
    fi

    # Check for wrong compiler (g++ instead of icpx)
    if grep -q "g++.*main.o" "$log_file"; then
        echo "$bench" >> errors_wrong_compiler.txt

    # Check for missing external headers
    elif grep -qi "CImg.h\|opencv\|png.h\|jpeg\|backprop.h file not found" "$log_file"; then
        echo "$bench" >> errors_missing_headers.txt

    # Check for shuffle functions
    elif grep -qi "no member named 'shuffle'" "$log_file"; then
        echo "$bench" >> errors_shuffle_functions.txt

    # Check for Intel extensions
    elif grep -qi "sycl::ext::intel::" "$log_file"; then
        echo "$bench" >> errors_intel_extensions.txt

    # Check for deprecated API
    elif grep -qi "deprecated\|access::target::local" "$log_file"; then
        echo "$bench" >> errors_deprecated_api.txt

    else
        echo "$bench" >> errors_other.txt
    fi
done

# Generate report
cat > "$OUTPUT" <<'EOFMD'
# Compilation Error Analysis

## Error Categories (Detailed)

### 1. Wrong Compiler (g++ instead of icpx)
**Count:** $(wc -l < errors_wrong_compiler.txt 2>/dev/null || echo 0)
**Severity:** HIGH - Easy batch fix
**Root Cause:** Makefiles use implicit rules that default to g++
**Fix:** Need to add explicit compilation rules or fix Makefile patterns

**Affected benchmarks:** See `errors_wrong_compiler.txt`

**Example error:**
```
g++    -c -o main.o main.cpp
main.cpp:6:10: fatal error: sycl/sycl.hpp: No such file or directory
```

**Fix strategy:**
- Option 1: Add explicit rule in Makefile: `%.o: %.cpp; $(CC) $(CFLAGS) -c $< -o $@`
- Option 2: Override make's built-in rules
- Option 3: Batch fix Makefiles with sed script

---

### 2. Missing External Headers/Libraries
**Count:** $(wc -l < errors_missing_headers.txt 2>/dev/null || echo 0)
**Severity:** MEDIUM - Need external dependencies
**Root Cause:** Benchmarks require external libraries (CImg, OpenCV, etc.)
**Fix:** Install dependencies or skip these benchmarks

**Affected benchmarks:** See `errors_missing_headers.txt`

---

### 3. Deprecated SYCL API
**Count:** $(wc -l < errors_deprecated_api.txt 2>/dev/null || echo 0)
**Severity:** MEDIUM - Need code updates
**Root Cause:** Using SYCL 1.2.1 APIs that are deprecated in SYCL 2020
**Fix:** Update to use local_accessor instead of accessor with access::target::local

**Example:**
```cpp
// Old (deprecated):
accessor<float, 1, sycl_read_write, access::target::local> s_data(size, cgh);

// New (SYCL 2020):
local_accessor<float, 1> s_data(size, cgh);
```

---

### 4. Missing Shuffle Functions
**Count:** $(wc -l < errors_shuffle_functions.txt 2>/dev/null || echo 0)
**Severity:** HIGH - API mismatch
**Root Cause:** Code uses shuffle functions that may not be available or have different names
**Fix:** Check SYCL 2020 sub-group collective function names

**Example error:**
```
error: no member named 'shuffle_down' in 'sycl::sub_group'
```

**Fix:** May need to use `sycl::shift_group_left` or update to correct API

---

### 5. Intel Extension Functions
**Count:** $(wc -l < errors_intel_extensions.txt 2>/dev/null || echo 0)
**Severity:** MEDIUM - Vendor-specific
**Root Cause:** Uses Intel-specific extensions that may not be available
**Fix:** Check if extensions are available in current compiler version

---

### 6. Other Errors
**Count:** $(wc -l < errors_other.txt 2>/dev/null || echo 0)
**Severity:** VARIES - Need individual review
**Fix:** Manual investigation needed

---

## Priority Fix Order

1. **HIGHEST: Wrong Compiler (87 benchmarks)**
   - Batch-fixable with Makefile modifications
   - Would immediately fix ~66% of compile errors

2. **HIGH: Deprecated API**
   - Pattern-based fixes possible
   - Update accessor syntax

3. **MEDIUM: Shuffle Functions & Intel Extensions**
   - Need to check SYCL 2020 API changes
   - May need vendor-specific handling

4. **LOW: External Dependencies**
   - Skip or install required libraries

5. **VARIES: Other**
   - Individual investigation

EOFMD

# Expand variables in the markdown
eval "cat <<EOFMD2
$(<"$OUTPUT")
EOFMD2" > "$OUTPUT"

echo ""
echo "========================================="
echo "ERROR ANALYSIS COMPLETE"
echo "========================================="
echo ""
echo "Error Categories:"
echo "  Wrong compiler (g++):        $(wc -l < errors_wrong_compiler.txt 2>/dev/null || echo 0)"
echo "  Missing headers/libs:        $(wc -l < errors_missing_headers.txt 2>/dev/null || echo 0)"
echo "  Deprecated SYCL API:         $(wc -l < errors_deprecated_api.txt 2>/dev/null || echo 0)"
echo "  Shuffle functions:           $(wc -l < errors_shuffle_functions.txt 2>/dev/null || echo 0)"
echo "  Intel extensions:            $(wc -l < errors_intel_extensions.txt 2>/dev/null || echo 0)"
echo "  Other:                       $(wc -l < errors_other.txt 2>/dev/null || echo 0)"
echo ""
echo "Detailed report: $OUTPUT"
echo ""
echo "🎯 KEY FINDING: $(wc -l < errors_wrong_compiler.txt 2>/dev/null || echo 0) benchmarks failed just because they used g++ instead of icpx!"
echo "   Fixing this would increase success rate significantly."
