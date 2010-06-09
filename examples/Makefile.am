## Process this file with automake to produce Makefile.in

OSMGPSMAP_CFLAGS =          \
	$(GLIB_CFLAGS)          \
	$(GTK_CFLAGS)           \
	$(CAIRO_CFLAGS)         \
	$(SOUP22_CFLAGS)        \
    $(SOUP24_CFLAGS)

OSMGPSMAP_LIBS =            \
    $(GLIB_LIBS)            \
    $(GTK_LIBS)             \
    $(CAIRO_LIBS)           \
    $(SOUP22_LIBS)          \
    $(SOUP24_LIBS)

## Demo Application
noinst_PROGRAMS = mapviewer

mapviewer_SOURCES =         \
	mapviewer.c

mapviewer_CFLAGS =          \
    -I$(top_srcdir)/src     \
    $(WARN_CFLAGS)          \
    $(DISABLE_DEPRECATED)   \
    $(OSMGPSMAP_CFLAGS)

mapviewer_LDADD =           \
    $(OSMGPSMAP_LIBS)       \
	$(top_builddir)/src/libosmgpsmap.la

## Misc
EXTRA_DIST = poi.png mapviewer.ui mapviewer.js

