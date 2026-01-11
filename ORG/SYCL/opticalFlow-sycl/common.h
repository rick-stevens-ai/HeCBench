/* Copyright (c) 2022, NVIDIA CORPORATION. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

///////////////////////////////////////////////////////////////////////////////
// Header for common includes and utility functions
///////////////////////////////////////////////////////////////////////////////

#ifndef COMMON_H
#define COMMON_H

///////////////////////////////////////////////////////////////////////////////
// Common includes
///////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <memory.h>
#include <math.h>
#include <iostream>
#include <sycl/sycl.hpp>

//#include <helper_cuda.h>

///////////////////////////////////////////////////////////////////////////////
// Common constants
///////////////////////////////////////////////////////////////////////////////
const int StrideAlignment = 32;

///////////////////////////////////////////////////////////////////////////////
// Common functions
///////////////////////////////////////////////////////////////////////////////

// Align up n to the nearest multiple of m
inline int iAlignUp(int n, int m = StrideAlignment) {
  int mod = n % m;

  if (mod)
    return n + m - mod;
  else
    return n;
}

// round up n/m
inline int iDivUp(int n, int m) { return (n + m - 1) / m; }

// swap two values
template <typename T>
inline void Swap(T &a, T &b) {
  T t = a;
  a = b;
  b = t;
}

#ifndef checkCudaErrors
#define checkCudaErrors(err)  // SYCL version: no-op
#endif

#ifndef CHECK_ERROR
#define CHECK_ERROR(expr) expr  // SYCL version: simple pass-through
#endif

// SDK helper function wrappers
#include "shrUtils.h"

inline char* sdkFindFilePath(const char* filename, const char* executable_path) {
  return shrFindFilePath(filename, executable_path);
}

inline bool sdkLoadPPM4ub(const char* file, unsigned char** data,
                          unsigned int *w, unsigned int *h) {
  return shrLoadPPM4ub(file, data, w, h) != 0;
}

//Note: N must be a power of two
#endif
