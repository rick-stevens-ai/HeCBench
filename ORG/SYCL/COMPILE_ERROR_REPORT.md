# Compilation Error Analysis Report

**Date:** January 4, 2026
**Analyzed:** 188 failed benchmarks out of 444 total

---

## 🎯 Executive Summary

**Major Discovery:** 87 benchmarks (46% of failures) failed simply because they used `g++` instead of `icpx`. **This is the #1 fixable issue.**

### Quick Stats
- ✅ **Working:** 256/444 (57.7%)
- ❌ **Failed:** 188/444 (42.3%)
- 🔧 **Easy fixes:** ~102 benchmarks (87 compiler + 15 FP64)
- 📊 **Potential after easy fixes:** ~358/444 (80.6%)

---

## 📊 Error Categories (Detailed Analysis)

### 1. 🔴 Wrong Compiler - g++ instead of icpx (87 benchmarks)

**Priority:** CRITICAL - Batch fixable
**Impact:** Would fix 46% of current failures
**Difficulty:** Easy - Makefile modification

**Root Cause:**
Makefiles have `CC=icpx` defined, but use implicit rules that default to `g++` for C++ compilation.

**Example:**
```bash
# What happens:
g++    -c -o main.o main.cpp
main.cpp:6:10: fatal error: sycl/sycl.hpp: No such file or directory
```

**Affected Benchmarks:** (Sample - full list in `errors_wrong_compiler.txt`)
```
attentionMultiHead-sycl, attention-sycl, bilateral-sycl, bitpermute-sycl,
blas-gemmBatched-sycl, blas-gemmEx2-sycl, blas-gemmEx-sycl,
channelShuffle-sycl, channelSum-sycl, dwconv-sycl, entropy-sycl,
epistasis-sycl, flip-sycl, fsm-sycl, gabor-sycl, ga-sycl, gc-sycl...
```

**Fix Strategy:**
```bash
# Option 1: Add explicit rule to each Makefile
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Option 2: Batch fix with sed script (see below)
```

---

### 2. 🟡 Deprecated SYCL API (14 benchmarks)

**Priority:** HIGH - Pattern-based fix
**Impact:** 7% of failures
**Difficulty:** Medium - Code changes needed

**Root Cause:**
Using SYCL 1.2.1 accessor API that's deprecated in SYCL 2020.

**Example Error:**
```
warning: 'local' has been explicitly marked deprecated here
Use `local_accessor` instead
```

**Affected Benchmarks:**
```
axhelm-sycl, b+tree-sycl, cm-sycl, diamond-sycl, dwt2d-sycl,
gibbs-sycl, grep-sycl, heartwall-sycl, logan-sycl, matrixT-sycl,
quicksort-sycl, sc-sycl, srad-sycl, wlcpow-sycl
```

**Fix Pattern:**
```cpp
// OLD (SYCL 1.2.1):
accessor<float, 1, sycl_read_write, access::target::local> s_data(64, cgh);

// NEW (SYCL 2020):
local_accessor<float, 1> s_data(64, cgh);
```

---

### 3. 🟠 FP64 Not Supported (15 benchmarks)

**Priority:** HIGH - Batch fixable
**Impact:** 8% of failures
**Difficulty:** Easy - Use conversion script

**Affected Benchmarks:**
```
atomicPerf-sycl, ccsd-trpdrv-sycl, che-sycl, complex-sycl,
contract-sycl, convolution1D-sycl, cooling-sycl, coordinates-sycl,
crossEntropy-sycl, cross-sycl, d3q19-bgk-sycl, damage-sycl,
degrid-sycl, divergence-sycl, dp-sycl
```

**Fix:**
```bash
./convert_fp64_to_fp32.sh <benchmark-dir>
```

---

### 4. 🟡 Missing Headers from CUDA Directory (Multiple)

**Priority:** MEDIUM
**Impact:** Various
**Difficulty:** Easy - Fix include paths

**Pattern:**
```cpp
#include "ccs.h"  // Looking in current dir
// But file is at: ../../CUDA/ccs-cuda/ccs.h
```

**Fix:** Update Makefile include paths or copy headers

---

### 5. 🔵 Sub-group Shuffle Functions (2 benchmarks)

**Priority:** MEDIUM
**Impact:** Minimal
**Difficulty:** Medium - API changes

**Affected:** bsw-sycl, (1 other)

**Error:**
```cpp
error: no member named 'shuffle_down' in 'sycl::sub_group'
```

**Fix:** Update to SYCL 2020 collective functions

---

### 6. 🟢 Other Issues (58 benchmarks)

**Priority:** LOW - Varies
**Impact:** 31% of failures
**Difficulty:** Varies - Individual review needed

Mix of:
- Missing external libraries (CImg, OpenCV)
- Subdirectory structure issues
- Various syntax errors
- Intel extension issues

---

## 🎯 Recommended Fix Priority

### Phase 1: Quick Wins (102 benchmarks) 🚀

**Expected result:** 358/444 working (80.6%)

1. **Fix wrong compiler (87 benchmarks)**
   ```bash
   # Create Makefile fix script
   ./fix_makefile_compiler.sh
   ```

2. **Fix FP64 issues (15 benchmarks)**
   ```bash
   for bench in $(cat errors_fp64.txt | cut -d: -f2); do
       ./convert_fp64_to_fp32.sh $bench
   done
   ```

### Phase 2: Pattern Fixes (14 benchmarks)

**Expected result:** 372/444 working (83.8%)

3. **Update deprecated SYCL APIs (14 benchmarks)**
   - Pattern-based search and replace
   - Update accessor → local_accessor

### Phase 3: Individual Fixes (Remaining)

4. **Review and fix remaining 72 benchmarks individually**

---

## 🔧 Batch Fix Scripts

### Fix Makefile Compiler Issue

I'll create a script to fix all Makefiles that are missing explicit compilation rules:

```bash
#!/bin/bash
# fix_makefile_compiler.sh

for bench in $(cat errors_wrong_compiler.txt); do
    makefile="$bench/Makefile"
    if [ ! -f "$makefile" ]; then
        continue
    fi

    # Check if explicit %.o: %.cpp rule exists
    if ! grep -q "^%.o: %.cpp" "$makefile"; then
        echo "Fixing $bench..."

        # Add explicit rule before the clean target
        sed -i '/^clean:/i\
%.o: %.cpp\
\t$(CC) $(CFLAGS) -c $< -o $@\
' "$makefile"
    fi
done
```

---

## 📈 Projected Success Rate

| Phase | Benchmarks Fixed | Cumulative Working | Success Rate |
|-------|------------------|-------------------|--------------|
| Current | 0 | 256 | 57.7% |
| After Phase 1 | +102 | 358 | 80.6% |
| After Phase 2 | +14 | 372 | 83.8% |
| Optimistic (Phase 3) | +30-40 | 402-412 | 90-93% |

---

## 📁 Generated Files

- `errors_wrong_compiler.txt` - 87 benchmarks using g++
- `errors_fp64.txt` - 15 FP64 benchmarks
- `errors_deprecated_api.txt` - 14 deprecated API benchmarks
- `errors_shuffle_functions.txt` - 2 shuffle function issues
- `errors_intel_extensions.txt` - 1 Intel extension issue
- `errors_missing_headers.txt` - 1 missing external library
- `errors_other.txt` - 58 various issues
- `compilation_logs/` - Individual logs for all benchmarks

---

## 🎬 Next Actions

1. **Create and run the Makefile fix script** → +87 benchmarks
2. **Run FP64 conversion** → +15 benchmarks
3. **Update deprecated APIs** → +14 benchmarks
4. **Recompile and reassess**

**Total potential improvement:** From 256 → 372 working (57.7% → 83.8%)
