#ifndef UTILS_H
#define UTILS_H

#include <cstdlib>

void init_matrix(float *matrix, int64_t num_rows, int64_t num_cols, int64_t nnz)
{
  int64_t n = (int64_t)num_rows * num_cols;

  float *d = (float *) malloc(n * sizeof(float));

  srand(123);
  for (int64_t i = 0; i < n; i++) d[i] = (float)i;
  for (int64_t i = n; i > 0; i--) {
    int64_t a = i-1;
    int64_t b = rand() % i;
    if (a != b) {
      auto t = d[a];
      d[a] = d[b];
      d[b] = t;
    }
  }

  srand48(123);
  for (int64_t i = 0; i < num_rows; i++) {
    for (int64_t j = 0; j < num_cols; j++) {
      matrix[i*num_cols+j] = (d[i*num_cols+j] >= nnz) ? 0 : (float)(drand48()+1);
    }
  }

  free(d);
}

#endif
