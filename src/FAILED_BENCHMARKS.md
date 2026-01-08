# Failed SYCL Benchmarks - Debug List

**Total Failed:** 43 benchmarks need fixing
**Total No Binary:** 8 benchmarks need investigation

## Priority 1: No Binary Produced (8) - EASIEST TO FIX
These compiled but produce different binary names:

1. amgmk-sycl - produces "AMGMk" instead of "main"
2. b+tree-sycl - check Makefile target
3. face-sycl - check Makefile target
4. grep-sycl - check Makefile target
5. haccmk-sycl - check Makefile target
6. heat-sycl - check Makefile target
7. hybridsort-sycl - check Makefile target
8. srad-sycl - check Makefile target

**Fix:** Simply rename binary or update Makefile

## Priority 2: Deprecated SYCL API (7) - STRAIGHTFORWARD FIX
Need to update from SYCL 1.2.1 to SYCL 2020 syntax:

1. axhelm-sycl - local accessor syntax
2. convolutionDeformable-sycl - deprecated accessor
3. frechet-sycl - deprecated API
4. rsmt-sycl - local accessor
5. shuffle-sycl - deprecated syntax
6. stsg-sycl - accessor syntax
7. tensorT-sycl - local accessor

**Fix:** Replace `accessor<T, 1, sycl_read_write, access::target::local>` 
with `local_accessor<T, 1>`

## Priority 3: Namespace Issues (3) - QUICK FIX
Math function namespace resolution:

1. btree-sycl - std::fabs issue
2. gibbs-sycl - namespace conflict
3. rushlarsen-sycl - math function namespace

**Fix:** Use `::fabs()` or `sycl::fabs()` instead of `std::fabs()`

## Priority 4: Pointer/Template Issues (4) - MODERATE
Type conversion and template matching:

1. ans-sycl - get_pointer() conversion
2. ddbp-sycl - template matching
3. segment-reduce-sycl - pointer conversion
4. testSNAP-sycl - template issues

**Fix:** Use get_multi_ptr() or update type conversions

## Priority 5: Complex Issues (21) - REQUIRES INVESTIGATION

### Likely Fixable:
1. bsw-sycl
2. depixel-sycl
3. distort-sycl
4. fresnel-sycl
5. halo-finder-sycl
6. heartwall-sycl
7. is-sycl
8. leukocyte-sycl
9. logan-sycl
10. remap-sycl
11. sad-sycl
12. seam-carving-sycl
13. slit-sycl
14. slu-sycl
15. sobel-sycl
16. sparkler-sycl
17. sss-sycl
18. tonemapping-sycl
19. wlcpow-sycl
20. wsm5-sycl
21. xlqc-sycl

## Priority 6: No Makefile (8) - NEED TO CREATE
1. daphne-sycl
2. dwconv1d-sycl
3. hpl-sycl
4. logic-resim-sycl
5. mf-sgd-sycl
6. miniFE-sycl
7. si-sycl
8. snicit-sycl

**Fix:** Create Makefile based on other benchmarks

---

## Debugging Strategy

### Phase 1: Quick Wins (18 benchmarks - 30 mins)
1. Fix "no binary" issues (8) - just rename/check Makefiles
2. Fix deprecated API (7) - search & replace
3. Fix namespace issues (3) - simple code changes

### Phase 2: Moderate Fixes (4 benchmarks - 1 hour)
4. Fix pointer/template issues

### Phase 3: Deep Investigation (21 benchmarks - 2-3 hours)
5. Investigate complex issues one by one

### Phase 4: Create Makefiles (8 benchmarks - 1 hour)
6. Create missing Makefiles

**Estimated Total Time:** 4-5 hours to fix all
**Expected Success Rate:** 80-90% (36-41 additional working benchmarks)

