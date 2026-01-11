#===============================================================================
# Top-level Makefile wrapper for logic-resim-sycl
#===============================================================================

export CC = icpx

.PHONY: all clean

all:
	cd Simulation && $(MAKE) CC=$(CC)

clean:
	cd Simulation && $(MAKE) clean

run: all
	$(LAUNCHER) ./Simulation/main
