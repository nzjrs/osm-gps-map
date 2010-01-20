defsdir = $(datadir)/pygtk/2.0/defs
defs_DATA = osmgpsmap.defs

INCLUDES = \
	$(OSMGPSMAP_CFLAGS) \
	$(GTK_CFLAGS) \
	$(PYGTK_CFLAGS) \
	$(PYTHON_INCLUDES)

pkgpythondir = $(pyexecdir)/gtk-2.0
pkgpyexecdir = $(pyexecdir)/gtk-2.0

pkgpyexec_LTLIBRARIES = osmgpsmap.la
osmgpsmap_la_LDFLAGS = -module -avoid-version -export-symbols-regex initosmgpsmap
osmgpsmap_la_LIBADD = $(OSMGPSMAP_LIBS) $(GTK_LIBS) $(PYGTK_LIBS)
osmgpsmap_la_SOURCES = osmgpsmapmodule.c osmgpsmap.c
#nodist_osmgpsmap_la_SOURCES = osmgpsmap.c

CLEANFILES = osmgpsmap.c
EXTRA_DIST = setup.py osmgpsmap.override $(defs_DATA)

osmgpsmap.c: osmgpsmap.defs osmgpsmap.override
.defs.c:
	(cd $(srcdir)\
	 && pygtk-codegen-2.0 \
		--register $(PYGTK_DEFSDIR)/gtk-types.defs \
	    --register $(PYGTK_DEFSDIR)/gdk-base-types.defs \
		--override $*.override \
		--prefix py$* $*.defs) > gen-$*.c \
	&& cp gen-$*.c $*.c \
	&& rm -f gen-$*.c

update-defs:
	$(PYGTK_H2DEF) ../src/osm-gps-map.h > osmgpsmap-new.defs
	

