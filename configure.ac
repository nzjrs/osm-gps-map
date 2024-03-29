AC_PREREQ([2.63])
AC_INIT([osm-gps-map],
        [1.2.1],
        [http://github.com/nzjrs/osm-gps-map/issues],
        [osm-gps-map])

AX_IS_RELEASE([git-directory])

AM_INIT_AUTOMAKE([1.11 foreign subdir-objects parallel-tests])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_SRCDIR([src/osm-gps-map.h])
AC_CONFIG_MACRO_DIR([m4])

AX_REQUIRE_DEFINED([GTK_DOC_CHECK])

AM_PROG_CC_C_O

AC_LIBTOOL_WIN32_DLL
AM_PROG_LIBTOOL

AM_SILENT_RULES([yes])

# Library dependencies
PKG_CHECK_MODULES(GLIB,     [glib-2.0 >= 2.16])
PKG_CHECK_MODULES(GTK,      [gtk+-3.0 >= 3.0])
PKG_CHECK_MODULES(CAIRO,    [cairo >= 1.8])
PKG_CHECK_MODULES(SOUP30,   [libsoup-3.0])

# The mapviewer demo also calls g_thread_init, so it needs to link against
# libgthread-2.0.
PKG_CHECK_MODULES(GTHREAD, [gthread-2.0])

AC_CHECK_FUNCS(gdk_event_get_scroll_deltas)

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
LT_VERSION_INFO=2:1:1
AC_SUBST(LT_VERSION_INFO)

GOBJECT_INTROSPECTION_REQS=0.10.0
GOBJECT_INTROSPECTION_CHECK([$GOBJECT_INTROSPECTION_REQS])

GTK_DOC_CHECK([1.12])

AC_OUTPUT([
osmgpsmap-1.0.pc
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
echo Introspection support : ${found_introspection}
echo gtk-doc documentation : ${enable_gtk_doc}
echo
