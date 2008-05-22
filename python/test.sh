#!/bin/sh
PYTHONPATH=/home/john/Albatross/branches/groundstation/osm-gps-map/python/.libs \
    python -c \
'import gtk
import osmgpsmap

gtk.threads_init()
a = gtk.Window()
a.connect("destroy", lambda x: gtk.main_quit())
m = osmgpsmap.GpsMap()

a.add(m)
a.show_all()

print "BBOX ",m.get_bbox()
print "COORDS ", m.get_co_ordinatites(0,0)

gtk.main()'
