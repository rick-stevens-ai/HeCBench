# SYCL Benchmark Test Results

**Date:** Fri Jan  9 11:36:28 AM CST 2026  
**Total Benchmarks Tested:** 379 / 414 expected

## Summary

| Status | Count | Percentage |
|--------|-------|------------|
| ✓ Passed | 38 | 10.0% |
| ✗ Failed | 12 | 3.1% |
| ☠ Crashed | 261 | 68.8% |
| ? No Data | 68 | 17.9% |

## Result Categories

### ✓ PASS (38 benchmarks)
Benchmarks that explicitly output "PASS" and completed successfully.

### ✗ FAIL (12 benchmarks)
Benchmarks that explicitly output "FAIL" or had runtime errors.

### ☠ CRASH (261 benchmarks)
Benchmarks that crashed, segfaulted, or timed out (>30s).

### ? NO_DATA (68 benchmarks)
Benchmarks that completed without errors but didn't output PASS/FAIL.

## Files Generated
- `benchmark_results.txt` - Complete results list
- `logs/` - Individual benchmark output logs (all 379 benchmarks)
- `crashes/` - Crash logs for failed benchmarks

---

**Note:** Some benchmarks may require specific input data files or GPU capabilities.
Benchmarks marked "NO_DATA" likely completed successfully but don't have explicit
validation output. Manual inspection recommended for critical benchmarks.

🤖 Generated with [Claude Code](https://claude.com/claude-code)
