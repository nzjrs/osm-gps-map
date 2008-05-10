## Process this file with automake to produce Makefile.in
## Created by Anjuta

SUBDIRS = src 

openstreetmap_gps_mapdocdir = ${prefix}/doc/openstreetmap-gps-map
openstreetmap_gps_mapdoc_DATA = \
	README\
	COPYING\
	AUTHORS\
	ChangeLog\
	INSTALL\
	NEWS

EXTRA_DIST = $(openstreetmap_gps_mapdoc_DATA)

# Copy all the spec files. Of cource, only one is actually used.
dist-hook:
	for specfile in *.spec; do \
		if test -f $$specfile; then \
			cp -p $$specfile $(distdir); \
		fi \
	done

