# External Library Setup for SYCL Benchmarks

## Overview

This document describes the external libraries that have been installed for the HeCBench SYCL benchmarks and how to use them in your benchmarks.

## Installed Libraries

All external libraries are centralized in: `/home/stevens/HeCBench/include/external/`

### 1. STB Image Library

**Files:**
- `stb_image.h` - Image loading (7,988 lines)
- `stb_image_write.h` - Image writing (1,724 lines)

**Source:** https://github.com/nothings/stb

**Description:** Single-header public domain libraries for loading and writing images.

**Supported formats:**
- Loading: JPEG, PNG, TGA, BMP, PSD, GIF, HDR, PIC, PNM
- Writing: PNG, BMP, TGA, JPEG, HDR

**Usage in Makefile:**
```makefile
CFLAGS := $(EXTRA_CFLAGS) -std=c++17 -Wall -I../../../include/external -fsycl
```

**Usage in code:**
```cpp
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
```

**Benchmarks using this:**
- sad-sycl
- seam-carving-sycl

### 2. Bitmap Image Library

**File:** `bitmap_image.hpp` (5,002 lines)

**Source:** https://github.com/ArashPartow/bitmap

**Description:** C++ Bitmap Library for reading, writing and manipulating bitmap images.

**Features:**
- Easy-to-use C++ interface
- RGB pixel manipulation
- Drawing primitives
- Image transformations

**Usage in Makefile:**
```makefile
CFLAGS := $(EXTRA_CFLAGS) -std=c++17 -Wall -I../../../include/external -fsycl
```

**Usage in code:**
```cpp
#include "bitmap_image.hpp"
```

## Adding External Libraries to Your Benchmark

To use these libraries in a new benchmark:

1. **Update your Makefile** to include the external library path:
   ```makefile
   CFLAGS := $(EXTRA_CFLAGS) -std=c++17 -Wall -I../../../include/external -fsycl
   ```

2. **Include the headers** in your source files:
   ```cpp
   #include "stb_image.h"
   #include "stb_image_write.h"
   #include "bitmap_image.hpp"
   ```

3. **Define implementation macros** (for STB libraries only, before first include):
   ```cpp
   #define STB_IMAGE_IMPLEMENTATION
   #include "stb_image.h"
   ```

## Installation Commands

If you need to reinstall or update these libraries:

```bash
# Create directory
mkdir -p /home/stevens/HeCBench/include/external

# Download STB libraries
cd /home/stevens/HeCBench/include/external
curl -O https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
curl -O https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h

# Download Bitmap library
curl -O https://raw.githubusercontent.com/ArashPartow/bitmap/master/bitmap_image.hpp
```

## Notes

- These are header-only libraries, no linking required
- STB libraries use a special pattern where you must define `STB_*_IMPLEMENTATION` in exactly one source file before including
- The external library directory is three levels up from individual benchmark directories (`../../../include/external/`)
- All libraries are public domain or MIT licensed
