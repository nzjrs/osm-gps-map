## Process this file with automake to produce Makefile.in
## Created by Anjuta

SUBDIRS = src

openstreetmap_gps_mapdocdir = ${prefix}/doc/openstreetmap-gps-map
openstreetmap_gps_mapdoc_DATA =                 \
	README                                      \
	COPYING                                     \
	AUTHORS                                     \
	ChangeLog                                   \
	INSTALL                                     \
	NEWS

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = osmgpsmap.pc

EXTRA_DIST = $(openstreetmap_gps_mapdoc_DATA)   \
	osmgpsmap.pc.in                             \
	python/Makefile.am                          \
	python/Makefile.in                          \
	python/acinclude.m4                         \
	python/ChangeLog                            \
	python/osmgpsmap.defs                       \
	python/setup.py                             \
	python/aclocal.m4                           \
	python/configure                            \
	python/osmgpsmapmodule.c                    \
	python/test.sh                              \
	python/AUTHORS                              \
	python/configure.ac                         \
	python/osmgpsmap.override                   \
	python/NEWS                                 \
	python/README

release:
	scp @PACKAGE@-@VERSION@.tar.gz john@open.grcnz.com:/srv/default/downloads/osm-gps-map/

