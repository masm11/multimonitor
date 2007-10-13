#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(multi-monitor, 0.1, masm@flowernet.gr.jp)
AM_INIT_AUTOMAKE([foreign dist-bzip2])
AC_CONFIG_SRCDIR([src/mccgraph.c])
AM_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CC_C99

if test "x$ac_cv_prog_cc_c99" = xno; then
  AC_MSG_ERROR([You need C99 compiler.])
fi

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([string.h unistd.h])

# Checks for library functions.
AC_CHECK_FUNCS([memset strstr])

# xfce and gtk
PKG_CHECK_MODULES(GTK, libxfce4panel-1.0 pangoft2 gtk+-2.0 >= 2.10 glib-2.0)

# installdirs
AC_SUBST(pkgdatadir, '$(datarootdir)/xfce4/panel-plugins')
AC_SUBST(pkglibexecdir, '$(libexecdir)/xfce4/panel-plugins')

AC_OUTPUT([Makefile src/Makefile])