#!/usr/bin/python

"""
 Copyright (C) Hadley Rich 2008 <hads@nice.net.nz>
 based on main.c - with thanks to John Stowers

 osm-gps.py is free software: you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by the
 Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 osm-gps.py is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along
 with this program.  If not, see <http://www.gnu.org/licenses/>.
"""

import os
import gtk.gdk
import gobject
import osmgpsmap
 
gtk.gdk.threads_init()

def print_tiles(osm):
    if osm.get_property('tiles-queued') != 0:
        print osm.get_property('tiles-queued'), 'tiles queued'
    return True

def zoom_in_clicked(button, osm):
    osm.set_zoom(osm.get_property('zoom') + 1)
 
def zoom_out_clicked(button, osm):
    osm.set_zoom(osm.get_property('zoom') - 1)

def home_clicked(button, osm):
    osm.set_mapcenter(-44.39, 171.25, 12)
 
def cache_clicked(button, osm):
    bbox = osm.get_bbox()
    osm.download_maps(
        *bbox,
        zoom_start=osm.get_property('zoom'),
        zoom_end=osm.get_property('max-zoom')
    )

def map_clicked(osm, event):
    latlon_entry.set_text(
        'latitude %s longitude %s' % (
            osm.get_property('latitude'),
            osm.get_property('longitude')
        )
    )
 
window = gtk.Window(gtk.WINDOW_TOPLEVEL)
window.set_default_size(500, 500)
window.connect('destroy', lambda x: gtk.main_quit())
window.set_title('OpenStreetMap GPS Mapper')

vbox = gtk.VBox(False, 0)
window.add(vbox)
vbox.show()
hbox = gtk.HBox(False, 0)

osm = osmgpsmap.GpsMap(
    tile_cache=os.path.expanduser('~/Maps/OpenStreetMap'),
    tile_cache_is_full_path=True
)
osm.connect('button_release_event', map_clicked)
latlon_entry = gtk.Entry()

zoom_in_button = gtk.Button(stock=gtk.STOCK_ZOOM_IN)
zoom_in_button.connect('clicked', zoom_in_clicked, osm)
zoom_out_button = gtk.Button(stock=gtk.STOCK_ZOOM_OUT)
zoom_out_button.connect('clicked', zoom_out_clicked, osm)
home_button = gtk.Button(stock=gtk.STOCK_HOME)
home_button.connect('clicked', home_clicked, osm)
cache_button = gtk.Button('Cache')
cache_button.connect('clicked', cache_clicked, osm)

vbox.pack_start(osm)
hbox.pack_start(zoom_in_button)
hbox.pack_start(zoom_out_button)
hbox.pack_start(home_button)
hbox.pack_start(cache_button)
vbox.pack_start(hbox, False)
vbox.pack_start(latlon_entry, False)

window.show_all()
 
gobject.timeout_add(500, print_tiles, osm)
 
gtk.main()

