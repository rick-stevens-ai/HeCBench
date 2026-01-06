#===============================================================================
# Top-level Makefile wrapper for daphne-sycl
#===============================================================================

export CC = icpx

.PHONY: all clean run

all:
	cd src/points2image && $(MAKE) CC=$(CC)

clean:
	cd src/points2image && $(MAKE) clean

run:
	cd src/points2image && $(MAKE) run
