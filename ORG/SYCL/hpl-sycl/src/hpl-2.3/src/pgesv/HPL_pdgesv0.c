/* 
 * -- High Performance Computing Linpack Benchmark (HPL)                
 *    HPL - 2.3 - December 2, 2018                          
 *    Antoine P. Petitet                                                
 *    University of Tennessee, Knoxville                                
 *    Innovative Computing Laboratory                                 
 *    (C) Copyright 2000-2008 All Rights Reserved                       
 *                                                                      
 * -- Copyright notice and Licensing terms:                             
 *                                                                      
 * Redistribution  and  use in  source and binary forms, with or without
 * modification, are  permitted provided  that the following  conditions
 * are met:                                                             
 *                                                                      
 * 1. Redistributions  of  source  code  must retain the above copyright
 * notice, this list of conditions and the following disclaimer.        
 *                                                                      
 * 2. Redistributions in binary form must reproduce  the above copyright
 * notice, this list of conditions,  and the following disclaimer in the
 * documentation and/or other materials provided with the distribution. 
 *                                                                      
 * 3. All  advertising  materials  mentioning  features  or  use of this
 * software must display the following acknowledgement:                 
 * This  product  includes  software  developed  at  the  University  of
 * Tennessee, Knoxville, Innovative Computing Laboratory.             
 *                                                                      
 * 4. The name of the  University,  the name of the  Laboratory,  or the
 * names  of  its  contributors  may  not  be used to endorse or promote
 * products  derived   from   this  software  without  specific  written
 * permission.                                                          
 *                                                                      
 * -- Disclaimer:                                                       
 *                                                                      
 * THIS  SOFTWARE  IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,  INCLUDING,  BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE UNIVERSITY
 * OR  CONTRIBUTORS  BE  LIABLE FOR ANY  DIRECT,  INDIRECT,  INCIDENTAL,
 * SPECIAL,  EXEMPLARY,  OR  CONSEQUENTIAL DAMAGES  (INCLUDING,  BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA OR PROFITS; OR BUSINESS INTERRUPTION)  HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT,  STRICT LIABILITY,  OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 * ---------------------------------------------------------------------
 */ 
/*
 * Include files
 */
#include "hpl.h"

#ifdef STDC_HEADERS
void HPL_pdgesv0
(
   HPL_T_grid *                     GRID,
   HPL_T_palg *                     ALGO,
   HPL_T_pmat *                     A
)
#else
void HPL_pdgesv0
( GRID, ALGO, A )
   HPL_T_grid *                     GRID;
   HPL_T_palg *                     ALGO;
   HPL_T_pmat *                     A;
#endif
{
/* 
 * Purpose
 * =======
 *
 * HPL_pdgesv0 factors a N+1-by-N matrix using LU factorization with row
 * partial pivoting.  The main algorithm  is the "right looking" variant
 * with look-ahead.  The  lower  triangular  factor is left unpivoted.
 * This function is suitable for matrix A with a large number of column
 * relatively to the number of rows.
 *
 * Arguments
 * =========
 *
 * GRID    (local input)                 HPL_T_grid *
 *         On entry,  GRID  points  to the data structure containing the
 *         process grid information.
 *
 * ALGO    (global input)                HPL_T_palg *
 *         On entry,  ALGO  points to  the data structure containing the
 *         algorithmic parameters.
 *
 * A       (local input/output)          HPL_T_pmat *
 *         On entry, A points to the data structure containing the local
 *         array information.
 *
 * ---------------------------------------------------------------------
 */ 
/*
 * .. Local Variables ..
 */
   HPL_T_panel                * * panel = NULL;
   HPL_T_UPD_FUN              HPL_pdupdate;
   int                        * ipid = NULL, * lindxA = NULL,
                             * lindxAU = NULL, * lpid = NULL,
                             * icurcol = NULL, ierr = 0, ii, in, j,
                             jb, jj = 0, jstart, k, mycol, myrow,
                             n, nb, nn, npcol, nprow, nq, tag;
   float                      pred;
   HPL_T_pmat                 mat;
/* ..
 * .. Executable Statements ..
 */
   if( ( n = A->n ) <= 0 ) return;
/*
 * Initializations
 */
   mat.mp  = A->mp;  mat.nq  = A->nq;  mat.info = 0;
   pred   = HPL_pdpanllN( GRID, ALGO, n, jstart = 0 );
/*
 * Allocate panel resources - panel array of panel pointers, arrays of
 * integers for row and column process mappings, arrays of local indexes,
 * panel buffers, probe function work array of doubles ...
 */
   nn = ( n >> 1 ); nn = ( nn > A->nb ? A->nb : nn );
   if( !( panel = (HPL_T_panel **)malloc( sizeof( HPL_T_panel * ) ) ) )
   { HPL_pabort( __LINE__, "HPL_pdgesv0", "Memory allocation failed" ); }
/*
 * Create and initialize the first panel
 */
   nb = A->nb; nq = A->nq;
   mycol = GRID->mycol; myrow = GRID->myrow;
   npcol = GRID->npcol; nprow = GRID->nprow;
/*
 * Find  local first and last columns of current panel - find the first
 * row for pivot search. If  the  column my process  owns is not within
 * the panel,  update only the first super-row and quit the main loop.
 * Otherwise compute the factorization and do not update.  Compute  the
 * non-pivoting panel factorization and update.
 */
   tag = MSGID_BEGIN_FACT;
   HPL_psdpanel_new( GRID, ALGO, n, n+1, Mmin( n, nb ), A, 0, 0, tag,
                 &panel[0] );

   nn = n; j = 0; nb = panel[0]->nb;

   while( nn > 0 )
   {
      jb = Mmin( nn, nb );
      if( mycol == icurcol[jj] )
      {
         HPL_pdfact( panel[0] );
         if( ierr == 0 ) ierr = panel[0]->info;
      }
      else
      {
         HPL_pdupdateTT( NULL, NULL, panel[0], -1 );
      }
      nn -= jb; j += jb;
/*
 * Release panel resources - re-initialize panel data structure
 */
      (void) HPL_psdpanel_free( panel[0] );
      HPL_psdpanel_init( GRID, ALGO, n, n+1, jb, A, j, j, tag, panel[0] );
   }
/*
 * Clean-up: restore panel
 */
   (void) HPL_psdpanel_disp( &panel[0] );
/*
 * Release panel resources and panel array
 */
   if( panel ) free( panel );
/*
 * End of HPL_pdgesv0
 */
}
