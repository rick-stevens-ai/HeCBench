SHELL = /bin/sh
.SUFFIXES: .cc .o

LULESH_EXEC = lulesh

MPI_INC = /usr/include/mpi
MPI_LIB = /usr/lib

USE_GPU = -DUSE_GPU
USE_MPI = -DUSE_MPI=0
SYCL_FLAGS = -fsycl -fsycl-targets=spir64_x86_64

CC = icpx

SOURCES2.0 = \
	lulesh.cc \
	lulesh-viz.cc \
	lulesh-util.cc \
	lulesh-init.cc \
	lulesh-comm.cc

OBJECTS2.0 = $(SOURCES2.0:.cc=.o)

cxx_compiler_flags = -std=c++17 -Wall -I. $(SYCL_FLAGS) -O3 $(USE_GPU) $(USE_MPI)

.cc.o: lulesh.h
	@echo "Building $@"
	$(CC) $(cxx_compiler_flags) -c $< -o $@

all: $(LULESH_EXEC)

$(LULESH_EXEC): $(OBJECTS2.0)
	@echo "Linking"
	$(CC) $(OBJECTS2.0) $(cxx_compiler_flags) -lm -o $@

clean:
	/bin/rm -rf *.o *~ $(LULESH_EXEC)
	/bin/rm -rf *.optrpt
	/bin/rm -rf core.*

run: $(LULESH_EXEC)
	./$(LULESH_EXEC) -s 100

tar: clean
	cd .. ; tar cvf lulesh-2.0.tar LULESH-2.0 ; cd LULESH-2.0
