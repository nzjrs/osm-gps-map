#!/bin/sh
PYTHONPATH=/home/john/Albatross/branches/groundstation/osm-gps-map/python/.libs \
    python -c \
'import gtk.gdk
import gobject
import osmgpsmap

gtk.gdk.threads_init()

def print_tiles(map):
    print map.get_property("tiles-queued")
    return True

a = gtk.Window()
a.connect("destroy", lambda x: gtk.main_quit())
m = osmgpsmap.GpsMap()

a.add(m)
a.show_all()

gobject.timeout_add(500, print_tiles, m)

gtk.main()'
