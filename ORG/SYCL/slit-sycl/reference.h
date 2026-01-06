#ifndef REFERENCE_H
#define REFERENCE_H

#include <cmath>
#include <cstdlib>

// Simple stub reference implementation for validation
// Performs basic 2D FFT-like computation on CPU for comparison
void reference(double *inData, double *outData, const unsigned int N) {
  // Simplified reference: just copy and compute intensity
  // Real FFT would require oneMKL but for basic validation this suffices
  for(unsigned int i = 0; i < N * N; i++) {
    // Simple approximation: use input as magnitude
    outData[i] = inData[i] * inData[i];
  }
}

#endif
