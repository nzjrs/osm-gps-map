## Process this file with automake to produce Makefile.in

## Created by Anjuta

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
	osm-gps-map.c \
	osm-gps-map-osd.c \
	osm-gps-map-layer.c \
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
