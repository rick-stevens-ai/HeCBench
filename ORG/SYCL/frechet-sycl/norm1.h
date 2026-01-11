#include <sycl/sycl.hpp>
#include "types.h"

DOUBLE norm1(int i, int j, const DOUBLE *c1, const DOUBLE *c2)
{
  DOUBLE dist, diff; /* Temp variables for simpler computations */
  int k; /* Index for iterating over dimensions */

  /* Initialise distance */
  dist = 0.0;

  for (k = 0; k < n_d; k++)
  {
    /*
     * Compute the distance between the k-th component of the i-th point
     * of the 1st curve and the k-th component of the j-th point of the
     * 2nd curve.
     *
     * Notice the 1-offset added for better readability (as in [1]).
     */
    diff = *(c1 + (i - 1)*n_d + k) - *(c2 + (j - 1)*n_d + k);
    /* Increment the accumulator variable with the absolute distance */
    dist += fabs(diff);
  }

  return dist;
}

// Iterative version - fills DP matrix row by row
void iterative_norm1(int n_1, int n_2, DOUBLE *ca,
                     const DOUBLE *c1, const DOUBLE *c2)
{
  // Fill base case: (1,1)
  ca[0] = norm1(1, 1, c1, c2);

  // Fill first column: (i, 1) for i > 1
  for (int i = 2; i <= n_1; i++) {
    DOUBLE *ca_ij = ca + (i - 1)*n_2;
    DOUBLE *ca_prev = ca + (i - 2)*n_2;
    *ca_ij = sycl::fmax(*ca_prev, norm1(i, 1, c1, c2));
  }

  // Fill first row: (1, j) for j > 1
  for (int j = 2; j <= n_2; j++) {
    DOUBLE *ca_ij = ca + (j - 1);
    DOUBLE *ca_prev = ca + (j - 2);
    *ca_ij = sycl::fmax(*ca_prev, norm1(1, j, c1, c2));
  }

  // Fill remaining cells: (i, j) for i,j > 1
  for (int i = 2; i <= n_1; i++) {
    for (int j = 2; j <= n_2; j++) {
      DOUBLE *ca_ij = ca + (i - 1)*n_2 + (j - 1);
      DOUBLE *ca_up = ca + (i - 2)*n_2 + (j - 1);      // (i-1, j)
      DOUBLE *ca_diag = ca + (i - 2)*n_2 + (j - 2);    // (i-1, j-1)
      DOUBLE *ca_left = ca + (i - 1)*n_2 + (j - 2);    // (i, j-1)

      *ca_ij = sycl::fmax(
          sycl::fmin(sycl::fmin(*ca_up, *ca_diag), *ca_left),
          norm1(i, j, c1, c2));
    }
  }
}

void distance_norm1 (
  sycl::nd_item<2> &item,
  int n_1, int n_2,
  DOUBLE *__restrict ca,
  const DOUBLE *__restrict c1,
  const DOUBLE *__restrict c2)
{
  // Only one work-item computes the entire matrix
  if (item.get_global_id(0) == 0 && item.get_global_id(1) == 0) {
    iterative_norm1(n_1, n_2, ca, c1, c2);
  }
}

