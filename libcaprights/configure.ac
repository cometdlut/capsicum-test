dnl -*-autoconf-*-
dnl libcaprights: Autoconf script
dnl (c) 2014 Google, Inc.


AC_PREREQ([2.69])
AC_INIT([libcaprights],[0.1.0])
AC_CONFIG_SRCDIR([capsicum.c])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE()

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CC_C99
AC_PROG_INSTALL
AC_PROG_LN_S

dnl Checks for libraries.
LT_INIT
AC_SUBST(LIBTOOL_DEPS)

dnl Checks for header files.
AC_HEADER_STDC
AC_HEADER_STDBOOL
AC_HEADER_ASSERT
AC_CHECK_HEADERS([limits.h sys/prctl.h sys/syscall.h linux/seccomp.h linux/capsicum.h linux/procdesc.h])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_PID_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT64_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
