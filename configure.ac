AC_PREREQ([2.62])
AC_INIT([osm-gps-map],
        [0.7.3],
        [http://github.com/nzjrs/osm-gps-map/issues],
        [osm-gps-map])
AM_INIT_AUTOMAKE([1.10])

AC_CONFIG_HEADER([config.h])
AC_CONFIG_SRCDIR([src/osm-gps-map.h])
AC_CONFIG_MACRO_DIR([m4])

AM_MAINTAINER_MODE

AM_PROG_CC_C_O

AC_LIBTOOL_WIN32_DLL
AM_PROG_LIBTOOL

m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

PKG_CHECK_MODULES(GLIB, [glib-2.0 >= 2.16])
PKG_CHECK_MODULES(GTK, [gtk+-2.0 >= 2.14])
PKG_CHECK_MODULES(CAIRO, [cairo >= 1.6])

PKG_CHECK_MODULES(SOUP24, libsoup-2.4,
    soup_ver=2.4, [
        PKG_CHECK_MODULES(SOUP22, libsoup-2.2, [
            soup_ver=2.2
            AC_DEFINE(USE_LIBSOUP22, 1, Use libsoup 2.2 instead of libsoup-2.4 to download tiles)],
                AC_ERROR([libsoup-2.4 (preferred) or libsoup-2.2 not found]))])

AC_MSG_CHECKING([for Win32])
case "$host" in
  *-*-mingw*)
    os_win32=yes
    ;;
  *)
    os_win32=no
    ;;
esac
AC_MSG_RESULT([$os_win32])
AM_CONDITIONAL(OS_WIN32, [test $os_win32 = yes])

# Before making a release, the LT_VERSION_INFO string should be modified.
# The string is of the form C:R:A.
# - If interfaces have been changed or added, but binary compatibility has
#   been preserved, change to C+1:0:A+1
# - If binary compatibility has been broken (eg removed or changed interfaces)
#   change to C+1:0:0
# - If the interface is the same as the previous version, change to C:R+1:A
LT_VERSION_INFO=2:1:0
AC_SUBST(LT_VERSION_INFO)

GTK_DOC_CHECK([1.12])
GOBJECT_INTROSPECTION_CHECK([0.6.7])
GNOME_COMPILE_WARNINGS([maximum])
GNOME_MAINTAINER_MODE_DEFINES

AC_OUTPUT([
osmgpsmap.pc
Makefile
src/Makefile
docs/Makefile
examples/Makefile
docs/reference/Makefile
docs/reference/version.xml
])

echo
echo $PACKAGE v$VERSION
echo
echo Prefix............... : $prefix
echo libsoup.............. : $soup_ver
echo Introspection support : ${found_introspection}
echo gtk-doc documentation : ${enable_gtk_doc}
echo
