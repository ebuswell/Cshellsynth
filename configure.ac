# configure.ac
# 
# Process this file with autoconf to produce a configure script.
#
# Copyright 2010 Evan Buswell
# 
# This file is part of Cshellsynth.
# 
# Cshellsynth is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
# 
# Cshellsynth is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with Cshellsynth.  If not, see <http://www.gnu.org/licenses/>.

AC_PREREQ([2.65])
AC_INIT([cshellsynth], [0.5501], [ebuswell@gmail.com])
AM_INIT_AUTOMAKE([-Wall foreign])
AC_CONFIG_SRCDIR([src/synth.c])
AC_CONFIG_HEADERS([src/config.h])
AC_CANONICAL_TARGET

# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL

AC_ARG_WITH([ruby],
	[AS_HELP_STRING([--with-ruby],
			[Build the ruby library])],
	[],
	[with_ruby=check])

RUBY=
AS_IF([test x$with_ruby == "xyes"],[AC_PATH_PROG([RUBY],[ruby])]
				   [AS_IF([test x$RUBY == "x"],[AC_MSG_FAILURE([Could not locate ruby and --with-ruby was indicated])])],
      [test x$with_ruby == "xcheck"],[AC_PATH_PROG([RUBY],[ruby])],
      [test x$with_ruby != "xno"],[AC_CHECK_FILE([$with_ruby],[RUBY=$with_ruby],
						 [AC_MSG_FAILURE([Ruby interpreter not found at the specified location])])])

AC_SUBST([RUBY_SUBDIR])

AS_IF([test x$RUBY != "x"],
      [AC_CONFIG_COMMANDS([ruby/Makefile], [
AS_IF([test $srcdir != "."],[cp $srcdir/ruby/*.c ./ruby])
cd ./ruby
AS_IF([test ${srcdir:0:1} != "/"],
      [$RUBY ../$srcdir/ruby/extconf.rb],
      [$RUBY $srcdir/ruby/extconf.rb])
cd ..
],[RUBY=$RUBY])]
      [RUBY_SUBDIR=ruby])

# Checks for libraries.
PKG_CHECK_MODULES([JACK], [jack], [HAVE_JACK=true], [AC_MSG_ERROR([libjack is required to build AC_PACKAGE_NAME])])
PKG_CHECK_MODULES([SNDFILE], [sndfile >= 1.0.2], [HAVE_SNDFILE=true], [AC_MSG_ERROR([libsndfile [(>= 1.0.2)] is required to build AC_PACKAGE_NAME])])

# Checks for language features.
AC_LANG([C])

AC_CHECK_SIZEOF([long])

# Checks for architecture.
AS_CASE([$target_cpu],
[i?86], [AC_DEFINE([ARCH_X86], [1], [Define if compiling for an x86 or x86_64/amd64 target.])],
[x86_64], [AC_DEFINE([ARCH_X86], [1], [Define if compiling for an x86 or x86_64/amd64 target.])
	   AC_DEFINE([ARCH_X86_64], [1], [Define if compiling for an x86_64/amd64 target.])],
[AC_MSG_ERROR([Only compiles for x86 and x86_64 cpus right now.  If you create atomic*
headers for your architecture you should be able to fix this.  The Linux Kernel is a good
place to start.])])

# Checks for header files.
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([malloc.h stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([atexit floor memset pow])

AC_CONFIG_FILES([Makefile
                 src/Makefile])

AC_OUTPUT
