#ifndef HPL_PFACT_H
#define HPL_PFACT_H

#include "hpl_misc.h"
#include "hpl_pmisc.h"
#include "hpl_grid.h"
#include "hpl_panel.h"

/*
 * ---------------------------------------------------------------------
 * #define macro constants
 * ---------------------------------------------------------------------
 */
#define    PFACT_LEFT_LOOKING   0
#define    PFACT_RIGHT_LOOKING  1
#define    PFACT_CROUT          2

/*
 * ---------------------------------------------------------------------
 * Function prototypes
 * ---------------------------------------------------------------------
 */

void            HPL_pdfact
(
   HPL_T_panel *
);

void            HPL_psfact
(
   HPL_T_panel *
);

float           HPL_pdpanllN
(
   HPL_T_grid *,
   HPL_T_palg *,
   const int,
   const int
);

float           HPL_pspanllN
(
   HPL_T_grid *,
   HPL_T_palg *,
   const int,
   const int
);

void            HPL_pdpanllT
(
   HPL_T_panel *
);

void            HPL_pspanllT
(
   HPL_T_panel *
);

void            HPL_pdpancrN
(
   HPL_T_panel *
);

void            HPL_pspancrN
(
   HPL_T_panel *
);

void            HPL_pdpancrT
(
   HPL_T_panel *
);

void            HPL_pspancrT
(
   HPL_T_panel *
);

void            HPL_pdpanrlN
(
   HPL_T_panel *
);

void            HPL_pspanrlN
(
   HPL_T_panel *
);

void            HPL_pdpanrlT
(
   HPL_T_panel *
);

void            HPL_pspanrlT
(
   HPL_T_panel *
);

#endif  /* HPL_PFACT_H */
