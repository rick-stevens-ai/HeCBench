#ifndef HPL_GRID_H
#define HPL_GRID_H

/*
 * HPL grid structure
 */
typedef struct HPL_S_grid
{
   MPI_Comm           all_comm;     /* grid communicator */
   MPI_Comm           row_comm;     /* row communicator */
   MPI_Comm           col_comm;     /* column communicator */
   int               *row_mapping;  /* row mapping */
   int               *col_mapping;  /* column mapping */
   int                nprow;        /* number of process rows */
   int                npcol;        /* number of process columns */
   int                myrow;        /* my row number */
   int                mycol;        /* my column number */
   int                order;        /* order of grid (0=Row-Major, 1=Column-Major) */
   int                rank;         /* my rank in the grid */
   int                size;         /* size of grid */
} HPL_T_grid;

/*
 * Grid function prototypes
 */
int             HPL_grid_init
(
   MPI_Comm,
   const int,
   const int,
   const int,
   HPL_T_grid *
);

int             HPL_grid_exit
(
   HPL_T_grid *
);

#if defined(HPL_DETAILED_TIMING)
int             HPL_grid_info
(
   const HPL_T_grid *,
   int *,
   int *,
   int *,
   int *
);
#endif

#endif  /* HPL_GRID_H */
