## Process this file with automake to produce Makefile.in

SUBDIRS = src examples docs
# website/ is GitHub Pages (Jekyll). Not in SUBDIRS or EXTRA_DIST so
# release tarballs stay the library (src, examples, gtk-doc).
AM_DISTCHECK_CONFIGURE_FLAGS =                  \
    --enable-gtk-doc                            \
    --enable-glade-catalog

ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

doc_DATA =                                      \
	README                                      \
	COPYING                                     \
	AUTHORS                                     \
	ChangeLog                                   \
	NEWS

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = osmgpsmap-1.0.pc

if ENABLE_GLADE_CATALOG
gladecatalog_DATA = osmgpsmap.xml
endif

# tests/test.py and osmgpsmap.xml ship in the tarball even if not installed.
EXTRA_DIST = tests/test.py osmgpsmap.xml

DISTCLEANFILES = gtk-doc.make

# Drop generated ChangeLog on VPATH distclean only.
distclean-local:
	if test "$(srcdir)" = "."; then :; else \
		rm -f ChangeLog; \
	fi

# Build ChangeLog from full git history for dist. NEWS is the curated release notes.
ChangeLog:
	$(AM_V_GEN)if test -d "$(top_srcdir)/.git"; then \
	  (cd "$(top_srcdir)" && git log --stat -M -C --name-status --date=short --no-color) > $@.tmp \
	  && mv -f $@.tmp $@ \
	  || ($(RM) $@.tmp; \
	      echo "Failed to generate ChangeLog, your ChangeLog may be outdated" >&2; \
	      (test -f $@ || echo "git-log is required to generate this file" >> $@)); \
	else \
	  test -f $@ || \
	  (echo "A git checkout and git-log is required to generate ChangeLog" >&2 && \
	   echo "A git checkout and git-log is required to generate this file" >> $@); \
	fi

.PHONY: ChangeLog

