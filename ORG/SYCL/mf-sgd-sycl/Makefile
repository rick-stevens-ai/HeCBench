#===============================================================================
# Top-level Makefile wrapper for mf-sgd-sycl
#===============================================================================

export CC = icpx

.PHONY: all clean

all:
	cd src && $(MAKE) CC=$(CC)

clean:
	cd src && $(MAKE) clean
