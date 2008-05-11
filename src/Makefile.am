## Process this file with automake to produce Makefile.in

## Created by Anjuta

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" $(OPENSTREETMAP_GPS_MAP_CFLAGS)

AM_CFLAGS =\
	 -Wall\
	 -g

bin_PROGRAMS = openstreetmap-gps-map

openstreetmap_gps_map_SOURCES = \
	main.c

openstreetmap_gps_map_DEPENDENCIES = \
	libosmgpsmap.la

openstreetmap_gps_map_LDFLAGS = \
	-losmgpsmap

openstreetmap_gps_map_LDADD = $(OPENSTREETMAP_GPS_MAP_LIBS)

lib_LTLIBRARIES = \
	libosmgpsmap.la

libosmgpsmap_la_SOURCES = \
	converter.c \
	osm-gps-map.c

