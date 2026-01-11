#===============================================================================
# Top-level Makefile wrapper for hpl-sycl (uses build script)
#===============================================================================

.PHONY: all clean run

all:
	bash build_sycl_examples.sh

clean:
	find . -type d -name "build" -exec rm -rf {} + 2>/dev/null || true
	rm -rf build_logs

run: all
	@echo "Building HPL benchmark..."
	cd src/hpl-2.3 && $(MAKE)
	@echo "Setting up and running HPL..."
	cd src/hpl-2.3/bin/intel64 && \
		cp ../../../../datafiles/HPL_small_gpu.dat HPL.dat && \
		export LD_LIBRARY_PATH=../../src/dpcpp/:$$LD_LIBRARY_PATH && \
		$(LAUNCHER) mpirun -n 1 ./xhpl
