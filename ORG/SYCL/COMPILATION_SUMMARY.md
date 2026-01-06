# SYCL Benchmark Compilation Summary

**Test Date:** Sun Jan  4 08:32:04 PM CST 2026
**Total Benchmarks:** 444

## Results Overview

| Status | Count | Percentage |
|--------|-------|------------|
| Already Compiled | 60 | 13.5% |
| Newly Compiled | 196 | 44.1% |
| **TOTAL WORKING** | **256** | **57.7**% |

## Failures by Category

| Error Type | Count | Priority | Fix Strategy |
|------------|-------|----------|--------------|
| FP64 not supported | 15 | **HIGH** | Batch fix with convert_fp64_to_fp32.sh |
| Include path issues | 0 | MEDIUM | Fix Makefile paths to ../../CUDA/ |
| Compile errors | 132 | MEDIUM | Individual investigation needed |
| Linker errors | 2 | MEDIUM | Library/dependency issues |
| Missing files | 0 | LOW | May need external data files |
| No Makefile | 9 | LOW | Incomplete benchmark |
| Timeout | 1 | LOW | Very slow compilation |
| Unknown/Other | 29 | LOW | Manual review needed |

## Prioritized Action Plan

### 🔴 Priority 1: FP64 Issues (15 benchmarks)
These can be batch-fixed by converting double→float. See `errors_fp64.txt`

**Action:**
```bash
# For each benchmark in errors_fp64.txt:
./convert_fp64_to_fp32.sh <benchmark-directory>
```

### 🟡 Priority 2: Include Path Issues (0 benchmarks)
Fix Makefile include paths. See `errors_include_path.txt`

**Action:** Update Makefiles to use `../../CUDA/` instead of `../`

### 🟡 Priority 3: Compile Errors (132 benchmarks)
Need individual investigation. See `errors_compile.txt` and logs in `compilation_logs/`

### 🔵 Priority 4: Other Issues
Linker errors, missing files, and unknown issues need case-by-case review.

## Files Generated

- `compilation_summary.txt` - One-line status per benchmark
- `error_categories.txt` - All errors with categories
- `errors_fp64.txt` - List of FP64 issues only
- `errors_include_path.txt` - List of include path issues
- `errors_compile.txt` - List of compile errors
- `errors_linker.txt` - List of linker errors
- `compilation_logs/` - Full compilation log for each benchmark

