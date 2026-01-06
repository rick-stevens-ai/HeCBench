#ifndef HPL_PANEL_H
#define HPL_PANEL_H

#include "hpl_pmisc.h"
#include "hpl_grid.h"

/*
 * HPL Panel data structure
 */
typedef struct HPL_S_panel
{
   struct HPL_S_grid   * grid;          /* ptr to the process grid */
   struct HPL_S_palg   * algo;          /* ptr to the algo parameters */
   struct HPL_S_pmat   * pmat;          /* ptr to the local array info */
   double              * A;             /* ptr to trailing part of A */
   float              * AFLOAT;        /* ptr to trailing part of A (single precision) */
   void                * L2;            /* ptr to received block */
   void                * L1;            /* ptr to previous block */
   void                * DGEMM;         /* ptr to trailing panel of A */
   int                 * DPIV;          /* ptr to panel pivots array */
   int                 * DINFO;         /* ptr to panel info array */
   int                * U;              /* ptr to rows to send */
   int                * IWORK;          /* work array */
   int                  nb;             /* panel width */
   int                  jb;             /* panel height */
   int                  m;              /* global # rows of trailing A */
   int                  n;              /* global # cols of trailing A */
   int                  ia;             /* global row index of trailing A */
   int                  ja;             /* global col index of trailing A */
   int                  mp;             /* local # rows of trailing A */
   int                  nq;             /* local # cols of trailing A */
   int                  ii;             /* local row index of trailing A */
   int                  jj;             /* local col index of trailing A */
   int                  lda;            /* local leading dim of array A */
   int                  ldl2;           /* local leading dim of array L2 */
   int                  prow;           /* proc. row owning 1st row of A */
   int                  pcol;           /* proc. col owning 1st col of A */
   int                  msgid;          /* message id for panel bcast */
   int                  info;           /* error info */
   int                  SRCE;           /* rank of process broadcasting */
} HPL_T_panel;

/*
 * Prototypes
 */
void            HPL_pdpanel_new
(
   HPL_T_grid *,
   HPL_T_palg *,
   const int,
   const int,
   const int,
   HPL_T_pmat *,
   const int,
   const int,
   const int,
   HPL_T_panel * *
);

void            HPL_psdpanel_new
(
   HPL_T_grid *,
   HPL_T_palg *,
   const int,
   const int,
   const int,
   HPL_T_pmat *,
   const int,
   const int,
   const int,
   HPL_T_panel * *
);

int             HPL_pdpanel_init
(
   HPL_T_grid *,
   HPL_T_palg *,
   const int,
   const int,
   const int,
   HPL_T_pmat *,
   const int,
   const int,
   const int,
   HPL_T_panel *
);

int             HPL_psdpanel_init
(
   HPL_T_grid *,
   HPL_T_palg *,
   const int,
   const int,
   const int,
   HPL_T_pmat *,
   const int,
   const int,
   const int,
   HPL_T_panel *
);

int             HPL_pdpanel_disp
(
   HPL_T_panel * *
);

int             HPL_psdpanel_disp
(
   HPL_T_panel * *
);

int             HPL_pdpanel_free
(
   HPL_T_panel *
);

int             HPL_psdpanel_free
(
   HPL_T_panel *
);

#endif  /* HPL_PANEL_H */
