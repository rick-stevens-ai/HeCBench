#ifndef HPL_PMAT_H
#define HPL_PMAT_H

#include "hpl_misc.h"
#include "hpl_blas.h"
#include "hpl_auxil.h"

#include "hpl_pmisc.h"
#include "hpl_grid.h"
#include "hpl_comm.h"
#include "hpl_pauxil.h"
#include "hpl_panel.h"

/*
 * HPL matrix data structure
 */
typedef struct HPL_S_pmat
{
   double              * A;        /* pointer to local array */
   float              * AFLOAT;   /* pointer to local array (single precision) */
   int                  n;        /* global number of rows */
   int                  nb;       /* blocking factor */
   int                  mp;       /* local number of rows */
   int                  nq;       /* local number of columns */
   int                  info;     /* computational flag */
} HPL_T_pmat;

/*
 * Function prototypes
 */
void            HPL_pdmatgen
(
   const HPL_T_grid *,
   const int,
   const int,
   const int,
   double *,
   const int,
   const int
);

void            HPL_psmatgen
(
   const HPL_T_grid *,
   const int,
   const int,
   const int,
   float *,
   const int,
   const int
);

#endif  /* HPL_PMAT_H */
