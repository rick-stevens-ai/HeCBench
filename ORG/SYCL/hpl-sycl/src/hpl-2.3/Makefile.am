AUTOMAKE_OPTIONS = foreign subdir-objects
ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = hpl

hpl_SOURCES = \
    testing/ptest/HPL_pddriver.c \
    testing/ptest/HPL_pdinfo.c \
    testing/ptest/HPL_pdtest.c \
    src/auxil/HPL_abort.c \
    src/auxil/HPL_dlacpy.c \
    src/auxil/HPL_dlamch.c \
    src/auxil/HPL_dlange.c \
    src/auxil/HPL_dlaprnt.c \
    src/auxil/HPL_dlatcpy.c \
    src/auxil/HPL_fprintf.c \
    src/auxil/HPL_slacpy.c \
    src/auxil/HPL_slamch.c \
    src/auxil/HPL_slange.c \
    src/auxil/HPL_slaprnt.c \
    src/auxil/HPL_slatcpy.c \
    src/auxil/HPL_warn.c \
    src/blas/HPL_saxpy.c \
    src/blas/HPL_scopy.c \
    src/blas/HPL_sgemm.c \
    src/blas/HPL_sgemv.c \
    src/blas/HPL_sger.c \
    src/blas/HPL_sscal.c \
    src/blas/HPL_sswap.c \
    src/blas/HPL_strsm.c \
    src/blas/HPL_strsv.c \
    src/comm/HPL_1ring.c \
    src/comm/HPL_1rinM.c \
    src/comm/HPL_2ring.c \
    src/comm/HPL_2rinM.c \
    src/comm/HPL_bcast.c \
    src/comm/HPL_binit.c \
    src/comm/HPL_blong.c \
    src/comm/HPL_blonM.c \
    src/comm/HPL_bwait.c \
    src/comm/HPL_copyL.c \
    src/comm/HPL_packL.c \
    src/comm/HPL_recv.c \
    src/comm/HPL_sdrv.c \
    src/comm/HPL_send.c \
    src/dpcpp/dpcpp_dgemm.cpp \
    src/grid/HPL_all_reduce.c \
    src/grid/HPL_barrier.c \
    src/grid/HPL_broadcast.c \
    src/grid/HPL_grid_exit.c \
    src/grid/HPL_grid_info.c \
    src/grid/HPL_grid_init.c \
    src/grid/HPL_max.c \
    src/grid/HPL_min.c \
    src/grid/HPL_pnum.c \
    src/grid/HPL_reduce.c \
    src/grid/HPL_sum.c \
    src/panel/HPL_pdpanel_disp.c \
    src/panel/HPL_pdpanel_free.c \
    src/panel/HPL_pdpanel_init.c \
    src/panel/HPL_pdpanel_new.c \
    src/pauxil/HPL_indxg2l.c \
    src/pauxil/HPL_indxg2lp.c \
    src/pauxil/HPL_indxg2p.c \
    src/pauxil/HPL_indxl2g.c \
    src/pauxil/HPL_infog2l.c \
    src/pauxil/HPL_numroc.c \
    src/pauxil/HPL_numrocI.c \
    src/pauxil/HPL_pabort.c \
    src/pauxil/HPL_pdlamch.c \
    src/pauxil/HPL_pdlange.c \
    src/pauxil/HPL_pdlaprnt.c \
    src/pauxil/HPL_pwarn.c \
    src/pauxil/HPL_slaswp00N.c \
    src/pauxil/HPL_slaswp01N.c \
    src/pauxil/HPL_slaswp01T.c \
    src/pauxil/HPL_slaswp02N.c \
    src/pauxil/HPL_slaswp03N.c \
    src/pauxil/HPL_slaswp03T.c \
    src/pauxil/HPL_slaswp04N.c \
    src/pauxil/HPL_slaswp04T.c \
    src/pauxil/HPL_slaswp05N.c \
    src/pauxil/HPL_slaswp05T.c \
    src/pauxil/HPL_slaswp06N.c \
    src/pauxil/HPL_slaswp06T.c \
    src/pauxil/HPL_slaswp10N.c \
    src/pfact/HPL_dlocmax.c \
    src/pfact/HPL_dlocswpN.c \
    src/pfact/HPL_dlocswpT.c \
    src/pfact/HPL_pdfact.c \
    src/pfact/HPL_pdmxswp.c \
    src/pfact/HPL_pdpancrN.c \
    src/pfact/HPL_pdpancrT.c \
    src/pfact/HPL_pdpanllN.c \
    src/pfact/HPL_pdpanllT.c \
    src/pfact/HPL_pdpanrlN.c \
    src/pfact/HPL_pdpanrlT.c \
    src/pfact/HPL_pdrpancrN.c \
    src/pfact/HPL_pdrpancrT.c \
    src/pfact/HPL_pdrpanllN.c \
    src/pfact/HPL_pdrpanllT.c \
    src/pfact/HPL_pdrpanrlN.c \
    src/pfact/HPL_pdrpanrlT.c \
    src/pfact/HPL_slocmax.c \
    src/pfact/HPL_slocswpN.c \
    src/pfact/HPL_slocswpT.c \
    src/pgesv/HPL_equil.c \
    src/pgesv/HPL_logsort.c \
    src/pgesv/HPL_pdgesv0.c \
    src/pgesv/HPL_pdgesv.c \
    src/pgesv/HPL_pdgesvK1.c \
    src/pgesv/HPL_pdgesvK2.c \
    src/pgesv/HPL_pdlaswp00N.c \
    src/pgesv/HPL_pdlaswp00T.c \
    src/pgesv/HPL_pdlaswp01N.c \
    src/pgesv/HPL_pdlaswp01T.c \
    src/pgesv/HPL_pdtrsv.c \
    src/pgesv/HPL_pdupdateNN.c \
    src/pgesv/HPL_pdupdateNT.c \
    src/pgesv/HPL_pdupdateTN.c \
    src/pgesv/HPL_pdupdateTT.c \
    src/pgesv/HPL_perm.c \
    src/pgesv/HPL_pipid.c \
    src/pgesv/HPL_plindx0.c \
    src/pgesv/HPL_plindx10.c \
    src/pgesv/HPL_plindx1.c \
    src/pgesv/HPL_rollN.c \
    src/pgesv/HPL_rollT.c \
    src/pgesv/HPL_spreadN.c \
    src/pgesv/HPL_spreadT.c \
    testing/matgen/HPL_dmatgen.c \
    testing/matgen/HPL_jumpit.c \
    testing/matgen/HPL_ladd.c \
    testing/matgen/HPL_lmul.c \
    testing/matgen/HPL_rand.c \
    testing/matgen/HPL_setran.c \
    testing/matgen/HPL_smatgen.c \
    testing/matgen/HPL_xjumpm.c \
    testing/pmatgen/HPL_pdmatgen.c \
    testing/ptimer/HPL_ptimer.c \
    testing/ptimer/HPL_ptimer_cputime.c \
    testing/ptimer/HPL_ptimer_walltime.c \
    testing/timer/HPL_timer.c \
    testing/timer/HPL_timer_cputime.c \
    testing/timer/HPL_timer_walltime.c

AM_CPPFLAGS = -I$(top_srcdir)/include -I$(MKLROOT)/include
AM_CFLAGS = $(AM_CPPFLAGS)
AM_CXXFLAGS = $(AM_CPPFLAGS) -fsycl

src_dpcpp_dpcpp_dgemm_o_CXXFLAGS = $(AM_CXXFLAGS) -UVERSION

hpl_LDADD = -L$(MKLROOT)/lib/intel64 -lmkl_sycl -lmkl_intel_ilp64 -lmkl_sequential -lmkl_core -lsycl -lOpenCL -lpthread -lm -ldl
