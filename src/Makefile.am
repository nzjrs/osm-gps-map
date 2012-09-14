## Process this file with automake to produce Makefile.in
lib_LTLIBRARIES = libosmgpsmap.la

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

## Shared library
libosmgpsmap_la_CFLAGS =    \
    $(WARN_CFLAGS)          \
    $(DISABLE_DEPRECATED)   \
    $(OSMGPSMAP_CFLAGS)     \
	-DG_LOG_DOMAIN=\"OsmGpsMap\"

if OS_WIN32
no_undefined = -no-undefined
endif

libosmgpsmap_la_LDFLAGS = \
	-version-info $(LT_VERSION_INFO)    \
	$(no_undefined)

libosmgpsmap_la_LIBADD =    \
    $(OSMGPSMAP_LIBS)

sources_private_h =         \
	converter.h             \
	osd-utils.h             \
	private.h

sources_public_h =          \
    osm-gps-map.h           \
    osm-gps-map-osd.h       \
    osm-gps-map-layer.h     \
    osm-gps-map-track.h     \
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
    osm-gps-map-point.c     \
    osm-gps-map-image.c     \
    osm-gps-map-source.c    \
    osm-gps-map-widget.c    \
    osm-gps-map-compat.c

libosmgpsmap_la_SOURCES =   \
	$(sources_public_h)     \
	$(sources_private_h)    \
	$(sources_c)

libosmgpsmapincdir = $(includedir)/osmgpsmap
libosmgpsmapinc_HEADERS = $(sources_public_h)

## Misc
CLEANFILES =

## GObject Introspection Support
-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = --add-include-path=$(srcdir) --add-include-path=$(builddir) --warn-all
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir)

if HAVE_INTROSPECTION
introspection_sources = $(sources_c) $(sources_public_h)

OsmGpsMap-@VERSION@.gir: libosmgpsmap.la
OsmGpsMap_@OGM_MAJOR_MINOR@_gir_INCLUDES = GObject-2.0 Gtk-3.0
OsmGpsMap_@OGM_MAJOR_MINOR@_gir_CFLAGS = $(OSMGPSMAP_CFLAGS)
OsmGpsMap_@OGM_MAJOR_MINOR@_gir_LIBS = libosmgpsmap.la
OsmGpsMap_@OGM_MAJOR_MINOR@_gir_FILES = $(introspection_sources)
OsmGpsMap_@OGM_MAJOR_MINOR@_gir_SCANNERFLAGS = --identifier-prefix=OsmGps
INTROSPECTION_GIRS += OsmGpsMap-@VERSION@.gir

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(gir_DATA) $(typelib_DATA)
endif
