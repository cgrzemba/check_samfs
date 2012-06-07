#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([FULL-PACKAGE-NAME], [VERSION], [BUG-REPORT-ADDRESS])
AC_CONFIG_SRCDIR([src/check_samfs.c])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lfsmgmt':
AC_CHECK_LIB([fsmgmt], [main])

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h])

# check for --with-samfs-inc
AC_MSG_CHECKING(for --with-samfs-inc)
AC_ARG_WITH(samfs-inc, AS_HELP_STRING([--with-samfs-inc=PATH],[SamFS pub include directory]),
[
  if test -e "$withval"/pub/mgmt/faults.h
  then
    AC_MSG_RESULT([using $withval])
    samfs_inc="-I$withval"
  else
    echo
    AC_MSG_ERROR([$withval not found])
  fi
],
[
  AC_MSG_RESULT(no)
  samfs_inc=""
  AC_MSG_ERROR([option --with-samfs-inc mandatory])
])
AC_SUBST([samfs_inc])
AC_SUBST([prefix])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
