#!/bin/bash

echo "Creating missing Makefiles..."
echo ""

# Benchmarks without Makefiles
BENCHMARKS="daphne-sycl dwconv1d-sycl hpl-sycl logic-resim-sycl mf-sgd-sycl miniFE-sycl si-sycl snicit-sycl"

CREATED=0
FAILED=0

for bench in $BENCHMARKS; do
    echo -n "Checking $bench... "
    
    if [ ! -d "$bench" ]; then
        echo "✗ Directory not found"
        ((FAILED++))
        continue
    fi
    
    cd "$bench"
    
    # Check if Makefile exists
    if [ -f "Makefile" ]; then
        echo "→ Already has Makefile"
        cd ..
        continue
    fi
    
    # Check for source files
    CPP_FILES=$(ls *.cpp 2>/dev/null | wc -l)
    
    if [ $CPP_FILES -eq 0 ]; then
        echo "✗ No .cpp files found"
        ((FAILED++))
        cd ..
        continue
    fi
    
    # List source files
    SOURCES=$(ls *.cpp 2>/dev/null | tr '\n' ' ')
    
    echo "✓ Found $CPP_FILES source files, creating Makefile..."
    
    # Create standard SYCL Makefile
    cat > Makefile << 'EOFMAKE'
#===============================================================================
# User Options
#===============================================================================

CC        = clang++
OPTIMIZE  = yes
DEBUG     = no
LAUNCHER  =

GPU       = yes
CUDA      = no
CUDA_ARCH = sm_70
HIP       = no
HIP_ARCH  = gfx908

#===============================================================================
# Program name & source code list
#===============================================================================

program = main

source = $(wildcard *.cpp)

obj = $(source:.cpp=.o)

#===============================================================================
# Sets Flags
#===============================================================================

CFLAGS := $(EXTRA_CFLAGS) -std=c++17 -Wall -fsycl --gcc-toolchain=$(GCC_TOOLCHAIN)

ifeq ($(VENDOR), AdaptiveCpp)
    CFLAGS_TMP := $(CFLAGS)
    CFLAGS = $(filter-out -fsycl, $(CFLAGS_TMP))
endif

LDFLAGS = 

ifeq ($(CUDA), yes)
  CFLAGS += -fsycl-targets=nvptx64-nvidia-cuda \
            -Xsycl-target-backend --cuda-gpu-arch=$(CUDA_ARCH)
endif

ifeq ($(HIP), yes)
  CFLAGS += -fsycl-targets=amdgcn-amd-amdhsa \
            -Xsycl-target-backend --offload-arch=$(HIP_ARCH) 
endif

ifeq ($(DEBUG),yes)
  CFLAGS  += -g -DDEBUG
  LDFLAGS += -g
endif

ifeq ($(OPTIMIZE),yes)
  CFLAGS += -O3
endif

ifeq ($(GPU),yes)
  CFLAGS +=-DUSE_GPU
endif

#===============================================================================
# Targets to Build
#===============================================================================

$(program): $(obj)
	$(CC) $(CFLAGS) $(obj) -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(program) $(obj)

run: $(program)
	$(LAUNCHER) ./$(program)

EOFMAKE
    
    echo "   Created Makefile with sources: $SOURCES"
    ((CREATED++))
    
    cd ..
done

echo ""
echo "========================================="
echo "Makefile Creation Summary"
echo "========================================="
echo "Created: $CREATED"
echo "Failed: $FAILED"
echo ""
