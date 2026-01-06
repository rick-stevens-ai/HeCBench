# Compiler and base flags
CC = icpx
OPTIMIZE = yes
DEBUG = no
LAUNCHER =

# SYCL configuration
ONEAPI_ROOT ?= /opt/intel/oneapi
SYCL_INCLUDE = -I$(ONEAPI_ROOT)/compiler/latest/include

# Program configuration
program = main
source = main.cpp
obj = $(source:.cpp=.o)

# Base flags
CFLAGS := $(EXTRA_CFLAGS) -std=c++17 -Wall -fsycl $(SYCL_INCLUDE)

# Device selection flags
ifeq ($(GPU),yes)
	CFLAGS += -DUSE_GPU
	ifeq ($(INTEL_GPU), yes)
		CFLAGS += -fsycl-targets=spir64_gen -Xsycl-target-backend "-device acm-g10"
	endif
	ifeq ($(CUDA), yes)
		CFLAGS += -fsycl-targets=nvptx64-nvidia-cuda -Xsycl-target-backend --cuda-gpu-arch=$(CUDA_ARCH)
	endif
	ifeq ($(HIP), yes)
		CFLAGS += -fsycl-targets=amdgcn-amd-amdhsa -Xsycl-target-backend --offload-arch=$(HIP_ARCH)
	endif
endif

# Debug and optimization flags
ifeq ($(DEBUG),yes)
	CFLAGS += -g -DDEBUG
	LDFLAGS += -g
endif

ifeq ($(OPTIMIZE),yes)
	CFLAGS += -O3
endif

# Build targets
$(program): $(obj)
	$(CC) $(CFLAGS) $(obj) -o $@ $(LDFLAGS)

%.o: %.cpp reference.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(program) $(obj)

run: $(program)
	$(LAUNCHER) ./$(program) 8192 10000 10 100

.PHONY: clean run
