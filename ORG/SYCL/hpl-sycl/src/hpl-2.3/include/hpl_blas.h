#ifndef HPL_BLAS_H
#define HPL_BLAS_H

/* MKL headers */
#include <mkl.h>

/* BLAS Level 1 routines */
#ifndef HPL_CALL_CBLAS
void sswap(const int*, float*, const int*, float*, const int*);
void dswap(const int*, double*, const int*, double*, const int*);
void sscal(const int*, const float*, float*, const int*);
void dscal(const int*, const double*, double*, const int*);
void scopy(const int*, const float*, const int*, float*, const int*);
void dcopy(const int*, const double*, const int*, double*, const int*);
void saxpy(const int*, const float*, const float*, const int*, float*, const int*);
void daxpy(const int*, const double*, const double*, const int*, double*, const int*);
float sdot(const int*, const float*, const int*, const float*, const int*);
double ddot(const int*, const double*, const int*, const double*, const int*);
#endif

/* BLAS Level 2 routines */
#ifndef HPL_CALL_CBLAS
void sgemv(const char*, const int*, const int*, const float*, const float*,
          const int*, const float*, const int*, const float*, float*, const int*);
void dgemv(const char*, const int*, const int*, const double*, const double*,
          const int*, const double*, const int*, const double*, double*, const int*);
void sger(const int*, const int*, const float*, const float*, const int*,
         const float*, const int*, float*, const int*);
void dger(const int*, const int*, const double*, const double*, const int*,
         const double*, const int*, double*, const int*);
void strsv(const char*, const char*, const char*, const int*, const float*,
          const int*, float*, const int*);
void dtrsv(const char*, const char*, const char*, const int*, const double*,
          const int*, double*, const int*);
#endif

/* BLAS Level 3 routines */
#ifndef HPL_CALL_CBLAS
void sgemm(const char*, const char*, const int*, const int*, const int*,
          const float*, const float*, const int*, const float*, const int*,
          const float*, float*, const int*);
void dgemm(const char*, const char*, const int*, const int*, const int*,
          const double*, const double*, const int*, const double*, const int*,
          const double*, double*, const int*);
void strsm(const char*, const char*, const char*, const char*, const int*,
          const int*, const float*, const float*, const int*, float*, const int*);
void dtrsm(const char*, const char*, const char*, const char*, const int*,
          const int*, const double*, const double*, const int*, double*, const int*);
#endif

#endif  /* HPL_BLAS_H */
