#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([cshellsynth], [0.01], [ebuswell@gmail.com])
AM_INIT_AUTOMAKE([-Wall foreign])
AC_CONFIG_SRCDIR([src/instrument.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_AWK
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL

# Checks for libraries.
PKG_CHECK_MODULES([JACK], [jack], [HAVE_JACK=true], [HAVE_JACK=false])
# PKG_CHECK_MODULES([FREEVERB3], [freeverb3], [HAVE_FREEVERB3=true], [HAVE_FREEVERB3=false])
PKG_CHECK_MODULES([SNDFILE], [sndfile >= 1.0.2], [HAVE_SNDFILE=true], [HAVE_SNDFILE=false])

# Checks for header files.
AC_CHECK_HEADERS([malloc.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_ALLOCA
AC_FUNC_MALLOC
AC_CHECK_FUNCS([floorf memset powf strerror])

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
