## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	$(GLIB_CFLAGS) \
	$(GTK_CFLAGS) \
	$(CAIRO_CFLAGS) \
	$(SOUP22_CFLAGS) $(SOUP24_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -g

CLEANFILES =

bin_PROGRAMS = mapviewer

mapviewer_SOURCES = \
	main.c

mapviewer_LDFLAGS = \
	-lgthread-2.0

mapviewer_LDADD = \
	$(GTK_LIBS) \
	libosmgpsmap.la

lib_LTLIBRARIES = \
	libosmgpsmap.la

libosmgpsmap_la_SOURCES = \
	converter.c \
	osm-gps-map-widget.c \
	osm-gps-map-source.c \
	osm-gps-map-osd.c \
	osm-gps-map-layer.c \
	osm-gps-map-track.c \
	osm-gps-map-point.c \
	osm-gps-map-image.c \
	osm-gps-map-compat.c \
	osd-utils.c

libosmgpsmap_la_CFLAGS = \
	-DG_LOG_DOMAIN=\"OsmGpsMap\"

if OS_WIN32
no_undefined = -no-undefined
endif

libosmgpsmap_la_LDFLAGS = \
	-version-info $(LT_VERSION_INFO)    \
	$(no_undefined)

libosmgpsmap_la_LIBADD = \
	$(GLIB_LIBS) \
	$(GTK_LIBS) \
	$(CAIRO_LIBS) \
	$(SOUP22_LIBS) $(SOUP24_LIBS)

libosmgpsmapincdir = $(includedir)/osmgpsmap
libosmgpsmapinc_HEADERS = \
	osm-gps-map.h \
	osm-gps-map-layer.h \
	osm-gps-map-osd.h

noinst_HEADERS = \
	converter.h \
	osd-utils.h \
	osm-gps-map-types.h

EXTRA_DIST = \
	poi.png

## GObject Introspection Support
-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = --add-include-path=$(srcdir)
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir)

if HAVE_INTROSPECTION
introspection_sources = $(libosmgpsmap_la_SOURCES)

OsmGpsMap-1.0.gir: libosmgpsmap.la
OsmGpsMap_1_0_gir_INCLUDES = GObject-2.0 Gtk-2.0
OsmGpsMap_1_0_gir_CFLAGS = $(INCLUDES)
OsmGpsMap_1_0_gir_LIBS = libosmgpsmap.la
OsmGpsMap_1_0_gir_FILES = $(introspection_sources)
INTROSPECTION_GIRS += OsmGpsMap-1.0.gir

girdir = $(datadir)/gir-1.0
dist_gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(dist_gir_DATA) $(typelib_DATA)
endif
