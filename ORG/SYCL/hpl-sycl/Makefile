#===============================================================================
# Top-level Makefile wrapper for hpl-sycl (uses build script)
#===============================================================================

.PHONY: all clean

all:
	bash build_sycl_examples.sh

clean:
	find . -type d -name "build" -exec rm -rf {} + 2>/dev/null || true
	rm -rf build_logs
