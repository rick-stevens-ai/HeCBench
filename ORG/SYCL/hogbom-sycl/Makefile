#===============================================================================
# User Options
#===============================================================================

# Compiler can be set below, or via environment variable
CC        = icpx
OPTIMIZE  = yes
DEBUG     = no
VERIFY    = no
LAUNCHER  =

GPU       = yes
CUDA      = no
CUDA_ARCH = sm_70
HIP       = no
HIP_ARCH  = gfx908
#GCC_TOOLCHAIN = "/auto/software/gcc/x86_64/gcc-9.1.0/"

#===============================================================================
# Program name & source code list
#===============================================================================

program = main


obj = main.o kernels.o reference.o timer.o

#===============================================================================
# Sets Flags
#===============================================================================

# Standard Flags
CFLAGS := $(EXTRA_CFLAGS) -std=c++17 -Wall -I../include  \
          -fsycl --gcc-toolchain=$(GCC_TOOLCHAIN)

ifeq ($(VENDOR), AdaptiveCpp)
    CFLAGS_TMP := $(CFLAGS)
    CFLAGS = $(filter-out -fsycl, $(CFLAGS_TMP))
endif

# Linker Flags
LDFLAGS = 

ifeq ($(CUDA), yes)
  CFLAGS += -fsycl-targets=nvptx64-nvidia-cuda \
            -Xsycl-target-backend --cuda-gpu-arch=$(CUDA_ARCH)
endif

ifeq ($(HIP), yes)
  CFLAGS += -fsycl-targets=amdgcn-amd-amdhsa \
	    -Xsycl-target-backend --offload-arch=$(HIP_ARCH) 
endif

# Debug Flags
ifeq ($(DEBUG),yes)
  CFLAGS  += -g -DDEBUG
  LDFLAGS += -g
endif

# Optimization Flags
ifeq ($(OPTIMIZE),yes)
  CFLAGS += -O3
endif

ifeq ($(GPU),yes)
  CFLAGS +=-DUSE_GPU
endif

ifeq ($(VERIFY),yes)
  CFLAGS += -DVERIFY
endif
#===============================================================================
# Targets to Build
#===============================================================================

$(program): $(obj)
	$(CC) $(CFLAGS) $(obj) -o $@ $(LDFLAGS)

main.o: main.cpp utils.h timer.h reference.h kernels.h
	$(CC) $(CFLAGS) -c $< -o $@

reference.o: reference.cpp reference.h 
	$(CC) $(CFLAGS) -c $< -o $@

timer.o: timer.cpp timer.h 
	$(CC) $(CFLAGS) -c $< -o $@

kernels.o: kernels.cpp kernels.h timer.h 
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(program) $(obj)

run: $(program)
	$(LAUNCHER) ./$(program) data/dirty_4096.img data/psf_4096.img 1000
