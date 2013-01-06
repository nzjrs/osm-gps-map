## Process this file with automake to produce Makefile.in

OSMGPSMAP_CFLAGS =          \
    $(GLIB_CFLAGS)          \
    $(GTK_CFLAGS)           \
    $(CAIRO_CFLAGS)         \
    $(SOUP24_CFLAGS)

OSMGPSMAP_LIBS =            \
    $(GLIB_LIBS)            \
    $(GTK_LIBS)             \
    $(CAIRO_LIBS)           \
    $(SOUP24_LIBS)

## Demo Application
if HAVE_GTHREAD
noinst_PROGRAMS = mapviewer
else
noinst_PROGRAMS =
endif

mapviewer_SOURCES =         \
    mapviewer.c

mapviewer_CFLAGS =          \
    -I$(top_srcdir)/src     \
    $(WARN_CFLAGS)          \
    $(DISABLE_DEPRECATED)   \
    $(OSMGPSMAP_CFLAGS)     \
    $(GTHREAD_CFLAGS)

mapviewer_LDADD =           \
    $(OSMGPSMAP_LIBS)       \
    $(GTHREAD_LIBS)         \
    $(top_builddir)/src/libosmgpsmap.la

## Misc
EXTRA_DIST = poi.png mapviewer.ui mapviewer.js README

