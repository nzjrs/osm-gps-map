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
noinst_PROGRAMS = mapviewer polygon editable_track

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
    $(top_builddir)/src/libosmgpsmap-1.0.la


polygon_SOURCES =         \
    polygon.c

polygon_CFLAGS =          \
    -I$(top_srcdir)/src     \
    $(WARN_CFLAGS)          \
    $(DISABLE_DEPRECATED)   \
    $(OSMGPSMAP_CFLAGS)     \
    $(GTHREAD_CFLAGS)

polygon_LDADD =           \
    $(OSMGPSMAP_LIBS)       \
    $(GTHREAD_LIBS)         \
    $(top_builddir)/src/libosmgpsmap-1.0.la


editable_track_SOURCES =         \
    editable_track.c

editable_track_CFLAGS =          \
    -I$(top_srcdir)/src     \
    $(WARN_CFLAGS)          \
    $(DISABLE_DEPRECATED)   \
    $(OSMGPSMAP_CFLAGS)     \
    $(GTHREAD_CFLAGS)

editable_track_LDADD =           \
    $(OSMGPSMAP_LIBS)       \
    $(GTHREAD_LIBS)         \
    $(top_builddir)/src/libosmgpsmap-1.0.la

## Misc
EXTRA_DIST = poi.png mapviewer.ui mapviewer.js README

