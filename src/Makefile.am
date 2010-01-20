## Process this file with automake to produce Makefile.in

## Created by Anjuta

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" $(OPENSTREETMAP_GPS_MAP_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -g

bin_PROGRAMS = mapviewer

mapviewer_SOURCES = \
	main.c

mapviewer_DEPENDENCIES = \
	libosmgpsmap.la

mapviewer_LDFLAGS = \
	-losmgpsmap -lgthread-2.0

mapviewer_LDADD = \
	$(OPENSTREETMAP_GPS_MAP_LIBS)

lib_LTLIBRARIES = \
	libosmgpsmap.la

libosmgpsmap_la_SOURCES = \
	converter.c \
	osm-gps-map.c

libosmgpsmap_la_CFLAGS = \
	-DG_LOG_DOMAIN=\"OsmGpsMap\"

if OS_WIN32
no_undefined = -no-undefined
endif

libosmgpsmap_la_LDFLAGS = \
	-version-info $(LT_VERSION_INFO)    \
	$(no_undefined)

libosmgpsmap_la_LIBADD = \
	$(OPENSTREETMAP_GPS_MAP_LIBS)

libosmgpsmapincdir = $(includedir)/osmgpsmap
libosmgpsmapinc_HEADERS = \
	osm-gps-map.h

noinst_HEADERS = \
	converter.h \
	osm-gps-map-types.h

EXTRA_DIST = \
	poi.png
