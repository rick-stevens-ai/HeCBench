#ifndef HPL_MISC_H
#define HPL_MISC_H

#include <mpi.h>

/* Define common macros */
#ifndef HPL_CALL_CBLAS
#define HPL_CALL_CBLAS
#endif

#define STDC_HEADERS
#ifdef STDC_HEADERS
#define STDC_ARGS(p)           p
#else
#define STDC_ARGS(p)
#endif

/* Common macro functions */
#define Mmin(a,b) (((a)>(b))?(b):(a))
#define Mmax(a,b) (((a)>(b))?(a):(b))
#define Mabs(a) (((a)>=0)?(a):-(a))

/* Error handling */
void HPL_abort
(
   int,
   const char *,
   const char *,
   ...
);

void HPL_pabort
(
   int,
   const char *,
   const char *,
   ...
);

/* Time functions */
#ifdef HPL_DETAILED_TIMING
#define HPL_TIMING_DEFINE(N)   double              (*N)[HPL_TIMING_N]
#define HPL_TIMING_ALLOCATE(N) (N) = (double(*)[HPL_TIMING_N])malloc(sizeof(double[p][HPL_TIMING_N]))
#define HPL_TIMING_FREE(N)     if (N) free(N)
#else
#define HPL_TIMING_DEFINE(N)   double              *N
#define HPL_TIMING_ALLOCATE(N) (N) = (double*)malloc(sizeof(double[p]))
#define HPL_TIMING_FREE(N)     if (N) free(N)
#endif

#endif  /* HPL_MISC_H */
