AC_INIT(osm-gps-map, 0.4.0)
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)

AC_CONFIG_SRCDIR(osmgpsmap.pc.in)
AM_CONFIG_HEADER(config.h)

AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC

AM_PROG_LIBTOOL

PKG_CHECK_MODULES(OPENSTREETMAP_GPS_MAP, [gtk+-2.0 libsoup-2.4 glib-2.0 >= 2.18])
AC_SUBST(OPENSTREETMAP_GPS_MAP_CFLAGS)
AC_SUBST(OPENSTREETMAP_GPS_MAP_LIBS)

AC_ARG_ENABLE(cairo, 
             [AC_HELP_STRING([--disable-cairo],
                             [Use Gdk instead of cairo to draw the map overlay])],
              enable_cairo="$enableval",
              enable_cairo="yes")

if test "x$enable_cairo" = "xyes"; then
    PKG_CHECK_MODULES(CAIRO,[cairo >= 1.8])
    AC_DEFINE(USE_CAIRO, 1, [Define if we're using cairo])
    OPENSTREETMAP_GPS_MAP_CFLAGS="$OPENSTREETMAP_GPS_MAP_CFLAGS $CAIRO_CFLAGS"
    OPENSTREETMAP_GPS_MAP_LIBS="$OPENSTREETMAP_GPS_MAP_LIBS $CAIRO_LIBS"
fi

CFLAGS="$CFLAGS -Wall -Wswitch-enum"

AC_OUTPUT([
osmgpsmap.pc
Makefile
src/Makefile
])

echo
echo $PACKAGE v$VERSION
echo
echo Prefix............... : $prefix
echo cairo................ : $enable_cairo
