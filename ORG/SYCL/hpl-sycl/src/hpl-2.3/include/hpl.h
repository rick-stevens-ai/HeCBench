#ifndef HPL_H
#define HPL_H

/* Standard headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <float.h>
#include <math.h>

/* HPL core headers - order matters */
#include "hpl_misc.h"
#include "hpl_blas.h"
#include "hpl_auxil.h"

/* HPL parallel headers - order matters */
#include "hpl_pmisc.h"
#include "hpl_grid.h"
#include "hpl_comm.h"
#include "hpl_pauxil.h"
#include "hpl_pmat.h"
#include "hpl_panel.h"
#include "hpl_pfact.h"
#include "hpl_pgesv.h"
#include "hpl_timer.h"
#include "hpl_matgen.h"
#include "hpl_pmatgen.h"
#include "hpl_ptimer.h"
#include "hpl_ptest.h"

#define MSGID_BEGIN_FACT   1001
#define MSGID_END_FACT     2000

#endif  /* HPL_H */
