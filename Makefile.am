## Process this file with automake to produce Makefile.in

SUBDIRS = src examples docs
DISTCHECK_CONFIGURE_FLAGS =                     \
    --enable-gtk-doc                            \
    --enable-maintainer-flags

ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

osm_gps_mapdocdir = ${prefix}/doc/osm-gps-map
osm_gps_mapdoc_DATA =                           \
	README                                      \
	COPYING                                     \
	AUTHORS                                     \
	ChangeLog                                   \
	INSTALL                                     \
	NEWS

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = osmgpsmap-1.0.pc

EXTRA_DIST = $(openstreetmap_gps_mapdoc_DATA)   \
	osmgpsmap-1.0.pc.in                         \
	gtk-doc.make		                        \
	m4/introspection.m4                         \
	m4/gtk-doc.m4

DISTCLEANFILES = gtk-doc.make

# ChangeLog generation from nautils
distclean-local:
	if test "$(srcdir)" = "."; then :; else \
		rm -f ChangeLog; \
	fi

ChangeLog:
	@echo Creating $@
	@if test -d "$(srcdir)/.git"; then \
	  (GIT_DIR=$(top_srcdir)/.git ./missing --run git log 0.7.3.. --stat -M -C --name-status  --date=short --no-color) | fmt --split-only > $@.tmp \
	  && mv -f $@.tmp $@ \
	  || ($(RM) $@.tmp; \
	      echo Failed to generate ChangeLog, your ChangeLog may be outdated >&2; \
	      (test -f $@ || echo git-log is required to generate this file >> $@)); \
	else \
	  test -f $@ || \
	  (echo A git checkout and git-log is required to generate ChangeLog >&2 && \
	  echo A git checkout and git-log is required to generate this file >> $@); \
	fi

release: dist
	scp @PACKAGE@-@VERSION@.tar.gz root@greenbirdsystems.com:/var/www/johnstowers.co.nz/files/osm-gps-map/

.PHONY: ChangeLog

