## Process this file with automake to produce Makefile.in

SUBDIRS = src examples docs
AM_DISTCHECK_CONFIGURE_FLAGS =                  \
    --enable-gtk-doc

ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

doc_DATA =                                      \
	README                                      \
	COPYING                                     \
	AUTHORS                                     \
	ChangeLog                                   \
	NEWS

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = osmgpsmap-1.0.pc

# Unit tests ship in the tarball (CI/git also run them in-tree).
EXTRA_DIST = tests/test.py

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

