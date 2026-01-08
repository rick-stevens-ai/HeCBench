# Debugging Progress

**Started:** 2026-01-08  
**Goal:** Fix 43+ failed benchmarks

## Completed Fixes

### Priority 1: No Binary Issues (8/8) ✓
1. ✓ amgmk-sycl - symlink to AMGMk
2. ✓ b+tree-sycl - symlink to b+tree.out
3. ✓ face-sycl - symlink to vj-gpu
4. ✓ grep-sycl - symlink to nfa
5. ✓ haccmk-sycl - symlink to haccmk
6. ✓ heat-sycl - symlink to heat
7. ✓ hybridsort-sycl - symlink to hybridsort
8. ✓ srad-sycl - symlink to srad

### Priority 2 & 3: Namespace Issues (2/3) ✓
1. ✓ axhelm-sycl - fixed std::fabs -> sycl::fabs + extracted BlasLapack
2. ✓ rushlarsen-sycl - fixed sqrt namespace in main.cpp only

### In Progress
3. btree-sycl - Intel extension issue (sycl::ext::intel::ctz)
4. gibbs-sycl - ambiguous function calls

## Summary
- **Total Fixed:** 10/43
- **Success Rate So Far:** 100% for attempted fixes
- **Remaining:** 33 benchmarks

## New Compilation Stats
- **Was:** 409/475 (86.1%)
- **Now:** 419/475 (88.2%)
- **Gained:** +10 working benchmarks
