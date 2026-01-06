#ifndef HPL_PMISC_H
#define HPL_PMISC_H

/*
 * HPL Algorithm parameters structure
 */
typedef struct HPL_S_palg
{
   int                nbdiv;     /* recursive stopping criteria */
   int                nbmin;     /* recursive stopping criteria */
   int                ndiv;      /* recursive stopping criteria */
   int                pfact;     /* panel fact variant */
   int                nbmax;     /* maximum block size */
   int                rfact;     /* recursive fact variant */
   int                bcast;     /* broadcast variant */
   int                depth;     /* look ahead depth */
   int                swap;      /* swapping algorithm */
   int                test;      /* testing */
} HPL_T_palg;

/*
 * HPL swap structure
 */
typedef struct HPL_S_swap
{
   int                IBEG;      /* Begining of the current row block */
   int                IEND;      /* End of the current row block */
   int                ICURR;     /* Current position in the row block */
   int                IPIV;      /* Current pivot */
   int                LINDXU;    /* Starting local current row index */
   int                LINDXAU;   /* Starting local current row index */
   int                MAXIDX;    /* Maximum local index */
   int               *LINDXA;    /* Original local indices */
   float            *A;         /* Original array */
   int               *U;         /* Column indices in U */
   int                IPLEN;     /* Length of the IPIV array */
   int               *IPIV2;     /* Row indices */
   int                NPCOL;     /* Number of process columns */
   int                NPROW;     /* Number of process rows */
   int                MYROW;     /* Current process row */
   int                MYCOL;     /* Current process column */
} HPL_T_SWAP;

/*
 * Update function type
 */
typedef void (*HPL_T_UPD_FUN)
(
   HPL_T_panel *,
   int *,
   HPL_T_panel *,
   const int
);

#endif  /* HPL_PMISC_H */
