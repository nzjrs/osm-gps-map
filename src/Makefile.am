## Process this file with automake to produce Makefile.in
lib_LTLIBRARIES = libosmgpsmap-1.0.la

OSMGPSMAP_CFLAGS =          \
	$(GLIB_CFLAGS)          \
	$(GTK_CFLAGS)           \
	$(CAIRO_CFLAGS)         \
    $(SOUP30_CFLAGS)

OSMGPSMAP_LIBS =            \
    $(GLIB_LIBS)            \
    $(GTK_LIBS)             \
    $(CAIRO_LIBS)           \
    $(SOUP30_LIBS)

## Shared library
libosmgpsmap_1_0_la_CFLAGS =    \
    $(WARN_CFLAGS)          \
    $(DISABLE_DEPRECATED)   \
    $(OSMGPSMAP_CFLAGS)     \
	-DG_LOG_DOMAIN=\"OsmGpsMap\"

if OS_WIN32
no_undefined = -no-undefined
endif

libosmgpsmap_1_0_la_LDFLAGS = \
	-version-info $(LT_VERSION_INFO)    \
	$(no_undefined)

libosmgpsmap_1_0_la_LIBADD =    \
    $(OSMGPSMAP_LIBS) \
    -lm

sources_private_h =         \
	converter.h             \
	osd-utils.h             \
	private.h

sources_public_h =          \
    osm-gps-map.h           \
    osm-gps-map-osd.h       \
    osm-gps-map-layer.h     \
    osm-gps-map-track.h     \
	osm-gps-map-polygon.h	\
    osm-gps-map-point.h     \
    osm-gps-map-image.h     \
    osm-gps-map-source.h    \
    osm-gps-map-widget.h    \
    osm-gps-map-compat.h

sources_c =                 \
    converter.c             \
    osd-utils.c             \
    osm-gps-map-osd.c       \
    osm-gps-map-layer.c     \
    osm-gps-map-track.c     \
	osm-gps-map-polygon.c	\
    osm-gps-map-point.c     \
    osm-gps-map-image.c     \
    osm-gps-map-source.c    \
    osm-gps-map-widget.c    \
    osm-gps-map-compat.c

libosmgpsmap_1_0_la_SOURCES =   \
	$(sources_public_h)     \
	$(sources_private_h)    \
	$(sources_c)

libosmgpsmapincdir = $(includedir)/osmgpsmap-1.0
libosmgpsmapinc_HEADERS = $(sources_public_h)

## Misc
CLEANFILES =

## GObject Introspection Support
-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = $(INTROSPECTION_FLAGS) --add-include-path=$(srcdir) --add-include-path=$(builddir) --warn-all --warn-error
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir) --includedir=$(builddir)

if HAVE_INTROSPECTION
introspection_sources = $(sources_c) $(sources_public_h)

OsmGpsMap-1.0.gir: libosmgpsmap-1.0.la
OsmGpsMap_1_0_gir_INCLUDES = GObject-2.0 Gtk-3.0
OsmGpsMap_1_0_gir_CFLAGS = $(OSMGPSMAP_CFLAGS) -I$(srcdir) -I$(builddir)
OsmGpsMap_1_0_gir_LIBS = libosmgpsmap-1.0.la
OsmGpsMap_1_0_gir_FILES = $(introspection_sources)
OsmGpsMap_1_0_gir_SCANNERFLAGS = --identifier-prefix=OsmGps --c-include "osm-gps-map.h"
INTROSPECTION_GIRS += OsmGpsMap-1.0.gir

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(gir_DATA) $(typelib_DATA)
endif
