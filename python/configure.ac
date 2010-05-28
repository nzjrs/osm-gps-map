AC_PREREQ([2.62])
AC_INIT([python-osmgpsmap],
        [0.7.0],
        [http://github.com/nzjrs/osm-gps-map/issues],
        [python-osmgpsmap])
AM_INIT_AUTOMAKE([1.10])

AC_CONFIG_SRCDIR([osmgpsmap.defs])
AC_CONFIG_MACRO_DIR([m4])

AM_MAINTAINER_MODE
AC_PROG_MAKE_SET

AC_PROG_CC
AM_PROG_CC_C_O

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

PKG_CHECK_MODULES(PYGTK, [pygtk-2.0 >= 2.10.0 pygobject-2.0 >= 2.16.0])

AC_MSG_CHECKING([for pygtk defs])
PYGTK_DEFSDIR=`$PKG_CONFIG --variable=defsdir pygtk-2.0`
AC_SUBST(PYGTK_DEFSDIR)
AC_MSG_RESULT($PYGTK_DEFSDIR)

AC_MSG_CHECKING([for codegen])
CODEGENDIR=`$PKG_CONFIG --variable codegendir pygobject-2.0`
AC_SUBST([CODEGENDIR])
AC_MSG_RESULT($CODEGENDIR)

if test -f ../src/libosmgpsmap.la; then
	#Because we are building against a non installed lib, the pkgconfig
	#file cannot tell us to also include/link gtk/soup, so we must manually do it
	PKG_CHECK_MODULES(GTK, [gtk+-2.0])
	PKG_CHECK_MODULES(SOUP24, [libsoup-2.4])

	#and we must link to the local lib
	OSMGPSMAP_CFLAGS="-I\$(top_srcdir)/../src/"
	OSMGPSMAP_LIBS="\$(top_builddir)/../src/libosmgpsmap.la"
else
	PKG_CHECK_MODULES(OSMGPSMAP, osmgpsmap)
fi

AC_OUTPUT([Makefile])

echo
echo $PACKAGE v$VERSION

echo
echo "Now type make to compile"
echo

