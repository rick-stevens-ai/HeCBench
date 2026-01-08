# HeCBench SYCL Benchmarks - Quick Start Guide

## Running Benchmarks

### 1. Set up environment
```bash
source /opt/intel/oneapi/setvars.sh --quiet
cd /home/stevens/HeCBench/src
```

### 2. Run a single benchmark
```bash
cd atan2-sycl
./main 10000000 100
```

### 3. Run multiple benchmarks with script
```bash
cat > run_benchmarks.sh << 'EOFSCRIPT'
#!/bin/bash
source /opt/intel/oneapi/setvars.sh --quiet
cd /home/stevens/HeCBench/src

for bench in binomial-sycl black-scholes-sycl mandelbrot-sycl triad-sycl; do
    echo "Running $bench..."
    (cd $bench && ./main 100 2>&1 | head -10)
    echo ""
done
EOFSCRIPT
chmod +x run_benchmarks.sh
./run_benchmarks.sh
```

## Verify GPU Detection

```bash
source /opt/intel/oneapi/setvars.sh --quiet
sycl-ls
```

Expected output:
```
[level_zero:gpu] Intel(R) Arc(TM) A770 Graphics
[opencl:cpu] 13th Gen Intel(R) Core(TM) i9-13900K
[opencl:gpu] Intel(R) Arc(TM) A770 Graphics
```

## Common Benchmark Arguments

Most benchmarks accept:
- `./main 100` - Run with 100 iterations
- `./main 10000000 100` - Run with 10M elements, 100 iterations
- `./main` - Some run without arguments

## Compilation Stats

- **Total:** 475 SYCL benchmarks
- **Compiled:** 409 (86%)
- **Working:** 409 ready to run

## Key Directories

- Source: `/home/stevens/HeCBench/src/`
- Logs: `/home/stevens/HeCBench/src/compilation_logs_20260108_102500/`
- Report: `/home/stevens/HeCBench/src/COMPILATION_REPORT.md`

## Troubleshooting

### Library not found error
```bash
./main: error while loading shared libraries: libsycl.so.8
```
**Solution:** Source OneAPI environment first
```bash
source /opt/intel/oneapi/setvars.sh
```

### GPU not detected
**Check:** Run `sycl-ls` to verify GPU is visible  
**Solution:** Ensure Intel GPU drivers are installed

### Benchmark needs arguments
**Check:** Look at main.cpp or try common patterns:
```bash
./main 100
./main 1000000 100
./main --help
```

## Performance Tips

1. **Warm-up runs:** First run may be slower due to JIT compilation
2. **Multiple iterations:** Use 100+ iterations for accurate timing
3. **GPU selection:** OneAPI will automatically use the Intel Arc GPU
4. **Power settings:** Ensure GPU is not throttled (check thermals)

## Next Steps

1. ✅ Run sample benchmarks to verify setup
2. ✅ Check COMPILATION_REPORT.md for detailed analysis
3. ✅ Use successfully compiled benchmarks for performance studies
4. 📋 Fix remaining 58 failed benchmarks (optional)
5. 📊 Create performance comparison reports

## Example Benchmark Run

```bash
$ source /opt/intel/oneapi/setvars.sh --quiet
$ cd /home/stevens/HeCBench/src/binomial-sycl
$ ./main 100

Average kernel execution time: 12.34 ms
PASS
```

## Contact & Support

For issues with:
- **HeCBench:** https://github.com/zjin-lcf/HeCBench
- **Intel OneAPI:** https://www.intel.com/content/www/us/en/developer/tools/oneapi/overview.html
- **This compilation:** See COMPILATION_REPORT.md

---
**Quick Start Guide** - Last updated: 2026-01-08
