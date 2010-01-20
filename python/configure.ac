AC_PREREQ(2.53)
AC_INIT(python-osmgpsmap, 0.5.0)
AC_CONFIG_SRCDIR(osmgpsmap.defs)
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)

AM_MAINTAINER_MODE
AC_PROG_MAKE_SET

AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC

AM_PROG_LIBTOOL

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AC_ARG_ENABLE(more-warnings,
	      [  --disable-more-warnings  Inhibit compiler warnings],
	      set_more_warnings=no)

if test "$GCC" = "yes" -a "$set_more_warnings" != "no"; then
	CFLAGS="$CFLAGS \
		-Wall -Wstrict-prototypes -Wmissing-declarations \
		-Wmissing-prototypes -Wnested-externs -Wpointer-arith"
fi


AM_PATH_PYTHON()
AM_CHECK_PYTHON_HEADERS(,AC_MSG_ERROR([install python-devel]))

PKG_CHECK_MODULES(PYGTK, [pygtk-2.0 >= 2.10.0])

AC_MSG_CHECKING(for pygtk defs)
PYGTK_DEFSDIR=`$PKG_CONFIG --variable=defsdir pygtk-2.0`
AC_SUBST(PYGTK_DEFSDIR)
AC_MSG_RESULT($PYGTK_DEFSDIR)

AC_MSG_CHECKING(for pygtk codegen)
PYGTK_CODEGEN="$PYTHON `$PKG_CONFIG --variable=codegendir pygtk-2.0`/codegen.py"
AC_SUBST(PYGTK_CODEGEN)
AC_MSG_RESULT($PYGTK_CODEGEN)

AC_MSG_CHECKING(for pygtk h2def)
PYGTK_H2DEF="$PYTHON `$PKG_CONFIG --variable=codegendir pygtk-2.0`/h2def.py"
AC_SUBST(PYGTK_H2DEF)
AC_MSG_RESULT($PYGTK_H2DEF)

if test -f ../src/libosmgpsmap.la; then
	#Because we are building against a non installed lib, the pkgconfig
	#file cannot tell us to also include/link gtk/soup, so we must manually do it
	PKG_CHECK_MODULES(OSMGPSMAP, [gtk+-2.0 libsoup-2.4])

	#and we must link to the local lib
	OSMGPSMAP_CFLAGS="$OSMGPSMAP_CFLAGS -I../src/"
	OSMGPSMAP_LIBS="$OSMGPSMAP_LIBS -L../src/.libs/ -losmgpsmap"
else
	PKG_CHECK_MODULES(OSMGPSMAP, osmgpsmap)
fi

AC_OUTPUT([Makefile])

echo
echo $PACKAGE v$VERSION

echo
echo "Now type make to compile"
echo

