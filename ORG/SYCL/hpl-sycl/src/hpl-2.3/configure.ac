AC_INIT([hpl-sycl], [1.0])
AC_CONFIG_SRCDIR([testing/ptest/HPL_pddriver.c])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([foreign subdir-objects])

# Checks for programs
AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB
AM_PROG_AR

# Check for Intel compiler and SYCL
AC_CHECK_PROG([ICPX], [icpx], [yes])
if test "x$ICPX" != "xyes"; then
    AC_MSG_ERROR([Intel C++ compiler (icpx) not found])
fi

# Check for MKL headers and environment
AC_MSG_CHECKING([for MKL environment])
if test "x$MKLROOT" = "x"; then
    AC_MSG_ERROR([MKLROOT environment variable not set])
else
    AC_MSG_RESULT([$MKLROOT])
fi

# Avoid VERSION macro conflict with MKL
AC_DEFINE([HPL_VERSION], ["1.0"], [HPL version number])

# Check for header files
AC_CHECK_HEADERS([mkl.h], [], [AC_MSG_ERROR([mkl.h not found])])

# Save and set flags for SYCL check
CXXFLAGS_save=$CXXFLAGS
LIBS_save=$LIBS
CXXFLAGS="$CXXFLAGS -fsycl -I$MKLROOT/include"
LIBS="-L$MKLROOT/lib/intel64 -lmkl_sycl -lmkl_intel_ilp64 -lmkl_sequential -lmkl_core -lsycl -lOpenCL -lpthread -lm -ldl"

# Check for MKL SYCL
AC_LANG_PUSH([C++])
AC_MSG_CHECKING([for MKL SYCL support])
AC_LINK_IFELSE(
    [AC_LANG_PROGRAM(
        [[
            #include <CL/sycl.hpp>
            #include <mkl.h>
        ]],
        [[
            sycl::queue Q;
            return 0;
        ]])],
    [AC_MSG_RESULT([yes])],
    [AC_MSG_RESULT([no])
     AC_MSG_ERROR([MKL SYCL support not found])])
AC_LANG_POP([C++])

# Restore original flags
CXXFLAGS=$CXXFLAGS_save
LIBS=$LIBS_save

# Output
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
