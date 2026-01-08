#ifndef REFERENCE_H
#define REFERENCE_H

#include <complex>
#include <cmath>

// Simple DFT-based reference implementation
// This is a CPU reference for validating the GPU FFT results
void reference(const double* input, double* output, int N) {
  std::complex<double>* temp = new std::complex<double>[N * N];

  // Convert real input to complex
  for (int i = 0; i < N * N; i++) {
    temp[i] = std::complex<double>(input[i], 0.0);
  }

  // Perform 2D DFT (slow but correct)
  std::complex<double>* result = new std::complex<double>[N * N];
  const double PI = 3.14159265358979323846;

  for (int ky = 0; ky < N; ky++) {
    for (int kx = 0; kx < N; kx++) {
      std::complex<double> sum(0.0, 0.0);
      for (int y = 0; y < N; y++) {
        for (int x = 0; x < N; x++) {
          double angle = -2.0 * PI * (kx * x + ky * y) / N;
          sum += temp[y * N + x] * std::complex<double>(cos(angle), sin(angle));
        }
      }
      result[ky * N + kx] = sum;
    }
  }

  // Compute magnitude squared
  for (int i = 0; i < N * N; i++) {
    output[i] = result[i].real() * result[i].real() +
                result[i].imag() * result[i].imag();
  }

  delete[] temp;
  delete[] result;
}

#endif
