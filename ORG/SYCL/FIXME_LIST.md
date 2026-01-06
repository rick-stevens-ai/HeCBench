# Comprehensive Fix List for SYCL Benchmarks
## Getting to 100% Working Benchmarks

**Current Status:** 383/444 working (86.3%)  
**Remaining:** 61 benchmarks to fix (13.7%)  
**Goal:** 444/444 (100%)

---

## Categories of Fixes Needed

### 1. FP64→FP32 Conversions (~15-20 benchmarks)
**Issue:** Intel Arc A770 GPU does not support double precision (FP64)  
**Solution:** Convert all `double` to `float`, add 'f' suffix to literals

**Known benchmarks needing FP64→FP32:**
- contract-sycl
- convolution2D-sycl  
- coordinates-sycl
- correlation-sycl
- covariance-sycl
- crs-sycl
- d2q9-bgk-sycl
- dct8x8-sycl
- ddbp-sycl (partial - needs verification)
- degrid-sycl (partial - needs verification)
- Any benchmark showing "Double type is not supported" error

**Fix procedure:**
```bash
# 1. Replace double with float
sed -i 's/\bdouble\b/float/g' *.cpp *.h

# 2. Add 'f' suffix to literals (use Python script)
python3 add_float_suffix.py

# 3. Update Makefile if PRECISION macro exists
sed -i 's/-DPRECISION=double/-DPRECISION=float/g' Makefile

# 4. Test compilation
make clean && make
```

---

### 2. Missing Headers/Source Files (~8-12 benchmarks)

#### A. Missing External Libraries
**Benchmarks:**
- **bm3d-sycl**: Needs CImg.h (C++ image processing library)
  - Solution: Install CImg or copy header to benchmark directory
  
- **snicit-sycl**: Needs CLI11/CLI11.hpp (command-line parser)
  - Solution: Download CLI11 from GitHub and add to include path
  
- **ccs-sycl**: Needs ccs.h
  - Solution: Check if file exists in CUDA version, copy if available

#### B. Missing CUDA Reference Files
**Benchmarks:**
- Many benchmarks reference `../benchmark-cuda/` for headers
- **Solution:** Update Makefile include paths to `../../CUDA/benchmark-cuda/`

**Pattern:**
```makefile
# Change from:
-I../benchmark-cuda

# To:
-I../../CUDA/benchmark-cuda
```

---

### 3. SYCL API Compatibility Issues (~5-8 benchmarks)

#### A. Deprecated/Removed SYCL Features
**bsw-sycl:**
- Error: `no member named 'shuffle_down' in 'sycl::sub_group'`
- Solution: Replace with `sycl::shuffle_down()` or equivalent group algorithm

**btree-sycl:**
- Error: `no member named 'ctz' in namespace 'sycl::ext::intel'`
- Solution: Replace with standard bit manipulation or portable alternative

#### B. SYCL 2020 vs Earlier Versions
Some benchmarks may use older SYCL API syntax that needs updating to SYCL 2020 standard.

---

### 4. Makefile Issues (~3-5 benchmarks)

#### A. Wrong Compiler
**Issue:** Some Makefiles use `g++` or `clang++` instead of `icpx`  
**Solution:**
```makefile
CC  = icpx
CXX = icpx
```

**Benchmarks potentially affected:**
- Any showing "sycl/sycl.hpp: No such file or directory" with g++

#### B. Missing Makefiles
**Solution:** Create wrapper Makefiles or adapt from similar benchmarks

---

### 5. Namespace/Function Name Issues (~2-5 benchmarks)

#### A. Incorrect std:: Usage
**Pattern:** `std::sinf()`, `std::expf()`, etc.  
**Fix:** Remove `std::` prefix - these are C functions, not C++ std functions

**Examples fixed:**
- degrid-sycl: `std::sinf` → `sinf`
- mt-sycl: `std::fabs` → `fabs`

#### B. Ambiguous Function Calls
**Pattern:** Compiler can't determine float vs double overload  
**Fix:** Add explicit `std::` prefix or use float-specific variants (`sinf` vs `sin`)

**Example fixed:**
- ddbp-sycl: `cos(theta)` → `std::cos(theta)`

---

### 6. Complex/Multi-File Projects (~3-5 benchmarks)

**Benchmarks:**
- **hpl-sycl**: Complex CMake project with multiple subdirectories
  - Uses `build_sycl_examples.sh` script
  - May require individual attention to each subproject

- **miniFE-sycl**: Large finite element mini-app
  - Already has working Makefile in src/
  - Wrapper created

- **logic-resim-sycl**: Logic simulation with multiple source directories
  - Already has working Makefile in Simulation/
  - Wrapper created

**Solution:** Create wrapper Makefiles that delegate to subdirectories

---

### 7. Backend/Driver Issues (~1-3 benchmarks)

**Issue:** Some benchmarks may fail with GPU driver or backend compiler issues  
**Symptoms:**
- "Backend compiler failed"
- "Device not found"
- JIT compilation failures

**Solutions:**
- Update Intel GPU drivers
- Check oneAPI compiler version compatibility
- May need to adjust target device flags in Makefile

---

## Priority Fix Order

### Phase 1: Quick Wins (Est. +10-15 benchmarks)
1. **Namespace fixes** - Simple sed replacements
2. **Makefile compiler fixes** - Change CC/CXX to icpx
3. **Include path fixes** - Update CUDA reference paths

### Phase 2: FP64→FP32 Conversions (Est. +15-20 benchmarks)
1. Create automated conversion script
2. Apply to all benchmarks with FP64 errors
3. Test each conversion

### Phase 3: External Dependencies (Est. +3-5 benchmarks)
1. Install/copy missing libraries (CImg, CLI11)
2. Copy missing headers from CUDA versions
3. Update include paths

### Phase 4: SYCL API Updates (Est. +5-8 benchmarks)
1. Replace deprecated shuffle operations
2. Update Intel extension calls
3. Modernize to SYCL 2020 API

### Phase 5: Complex Cases (Est. +3-5 benchmarks)
1. Multi-file projects
2. Backend compatibility issues
3. Architecture-specific problems

---

## Automated Fix Scripts

### Script 1: FP64→FP32 Converter
```python
#!/usr/bin/env python3
import re
import sys

def convert_to_fp32(filename):
    with open(filename, 'r') as f:
        content = f.read()
    
    # Replace double with float
    content = re.sub(r'\bdouble\b', 'float', content)
    
    # Add 'f' suffix to decimal literals
    content = re.sub(r'\b(\d+\.\d+)(?![fFlL])\b', r'\1f', content)
    
    with open(filename, 'w') as f:
        f.write(content)

if __name__ == '__main__':
    for filename in sys.argv[1:]:
        convert_to_fp32(filename)
        print(f"Converted {filename}")
```

### Script 2: Makefile Compiler Fixer
```bash
#!/bin/bash
# Fix Makefiles to use icpx compiler

for makefile in */Makefile; do
    if grep -q "CC.*=.*clang++" "$makefile" || grep -q "CC.*=.*g++" "$makefile"; then
        sed -i 's/^CC\s*=.*/CC        = icpx/' "$makefile"
        sed -i 's/^CXX\s*=.*/CXX       = icpx/' "$makefile"
        echo "Fixed: $makefile"
    fi
done
```

### Script 3: Include Path Updater
```bash
#!/bin/bash
# Update CUDA reference paths

for makefile in */Makefile; do
    if grep -q "\.\./.*-cuda" "$makefile"; then
        sed -i 's|-I\.\./\([^/]*\)-cuda|-I../../CUDA/\1-cuda|g' "$makefile"
        echo "Updated paths: $makefile"
    fi
done
```

---

## Testing Strategy

### Quick Test (Single Benchmark)
```bash
cd benchmark-sycl
make clean
timeout 60s make
[ -f main ] && echo "✓ SUCCESS" || echo "✗ FAILED"
```

### Batch Test (All Benchmarks)
```bash
#!/bin/bash
source /opt/intel/oneapi/setvars.sh --force > /dev/null 2>&1

success=0
failed=0

for dir in *-sycl; do
    cd "$dir"
    make clean > /dev/null 2>&1
    if timeout 60s make > /dev/null 2>&1; then
        ((success++))
    else
        ((failed++))
        echo "$dir" >> /tmp/failed_list.txt
    fi
    cd ..
done

echo "Results: $success/$((success+failed)) working"
```

---

## Known Working Benchmarks (383/444)

Recent fixes have brought us to 86.3% success rate. Key achievements:
- Session 1: Fixed 7 benchmarks (namespace & compiler issues)
- Session 2: Fixed 1 benchmark (FP32 conversion)
- Session 3: Fixed 3 benchmarks (new Makefiles)
- Session 4: Fixed 2 benchmarks (namespace & Makefile)
- Session 5: Fixed 2 benchmarks (FP32 conversions)

**Total improvement:** +125 benchmarks from starting point (+28.2%)

---

## Estimated Effort to 100%

- **Phase 1 (Quick Wins):** 1-2 hours
- **Phase 2 (FP64→FP32):** 2-3 hours
- **Phase 3 (Dependencies):** 1-2 hours
- **Phase 4 (SYCL API):** 3-4 hours
- **Phase 5 (Complex):** 4-6 hours

**Total estimated:** 11-17 hours of systematic work

With automated scripts and batch processing, this could be reduced to 6-10 hours.

---

## Next Steps

1. ✓ Create this comprehensive list
2. Run full benchmark test to get exact failure counts
3. Implement Phase 1 fixes (quick wins)
4. Create and test automated conversion scripts
5. Apply Phase 2 fixes (FP32 conversions) in batch
6. Address dependencies and API issues
7. Handle complex cases individually
8. Final verification test run
9. Document all changes

---

## Notes

- All fixes should maintain compatibility with the original benchmark intent
- Testing should verify not just compilation but also correctness
- Some benchmarks may have architecture-specific limitations
- GPU driver updates may be needed for some edge cases

---

**Last Updated:** 2026-01-06  
**Current Progress:** 383/444 (86.3%)  
**Target:** 444/444 (100%)
