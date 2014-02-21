# Process this file with autoconf to produce a configure script.
AC_INIT(cluster, 1.52)
AC_CONFIG_SRCDIR(src/cluster.c)
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS(config.h)

# Checks for programs.
AC_PROG_CC

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADER(float.h)
AC_CHECK_HEADER(math.h)
AC_CHECK_HEADER(stdio.h)
AC_CHECK_HEADER(stdlib.h)
AC_CHECK_HEADER(string.h)
AC_CHECK_HEADER(time.h)

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_LIB([m],[sqrt,exp,log])

# Check if we are building the GUI version or the command line version
AC_CONFIG_FILES([Makefile src/Makefile])
if test "$with_x" = no; then
  AC_MSG_NOTICE([Building command-line version of Cluster 3.0])
  AM_CONDITIONAL(MOTIF, false)
else
  AC_MSG_NOTICE([Building GUI version of Cluster 3.0 using Motif])
  AC_PROG_RANLIB
  AC_PATH_XTRA
  if test "$no_x" = "yes"; then
    AC_MSG_ERROR([Failed to locate the X11 include files and libraries.
Use --without-x if you want to build the command-line version of Cluster 3.0.])
  fi
  LIBS="$X_PRE_LIBS -lX11 $X_LIBS $X_EXTRA_LIB $LIBS"
  CPPFLAGS="$X_CFLAGS $CPPFLAGS"
  AC_CHECK_LIB(Xext, XShapeQueryVersion, [],
    [AC_MSG_ERROR([Failed to locate the Xext library.
Use --without-x if you want to build the command-line version of Cluster 3.0.])])
  AC_CHECK_LIB(Xt, XtMalloc, [],
    [AC_MSG_ERROR([Failed to locate the Xt library.
Use --without-x if you want to build the command-line version of Cluster 3.0.])])
  AC_CHECK_LIB(Xm, XmStringCreateSimple, [],
    [AC_MSG_ERROR([Failed to locate the Motif library.
Use --without-x if you want to build the command-line version of Cluster 3.0.])])
  AC_CHECK_HEADER(Xm/Xm.h, [],
    [AC_MSG_ERROR([Failed to locate the Motif header files.
Use --without-x if you want to build the command-line version of Cluster 3.0.
Otherwise, use CPPFLAGS to add the Motif header directory to the path.
For example, if Xm.h is in /usr/X11R6/include/Xm, use
./configure CPPFLAGS=-I/usr/X11R6/include
])])
  AM_CONDITIONAL(MOTIF, true)
  AC_CONFIG_FILES([X11/Makefile])
fi

AC_OUTPUT
