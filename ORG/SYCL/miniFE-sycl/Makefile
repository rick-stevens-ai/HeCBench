#===============================================================================
# Top-level Makefile wrapper for miniFE-sycl
#===============================================================================

export CC = icpx
export CXX = icpx

.PHONY: all clean

all:
	cd src && $(MAKE) CC=$(CC) CXX=$(CXX)

clean:
	cd src && $(MAKE) clean
