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

import sys
import os.path
import gtk.gdk
import gobject

#Try static lib first
mydir = os.path.dirname(os.path.abspath(__file__))
libdir = os.path.join(mydir, ".libs")
sys.path.insert(0, libdir)

import osmgpsmap
 
class UI(gtk.Window):
    def __init__(self):
        gtk.Window.__init__(self, gtk.WINDOW_TOPLEVEL)

        self.set_default_size(500, 500)
        self.connect('destroy', lambda x: gtk.main_quit())
        self.set_title('OpenStreetMap GPS Mapper')

        vbox = gtk.VBox(False, 0)
        self.add(vbox)
        vbox.show()
        hbox = gtk.HBox(False, 0)

        self.osm = osmgpsmap.GpsMap(
            tile_cache=os.path.expanduser('~/Maps/OpenStreetMap'),
            tile_cache_is_full_path=True
        )
        self.osm.connect('button_release_event', self.map_clicked)
        self.latlon_entry = gtk.Entry()

        zoom_in_button = gtk.Button(stock=gtk.STOCK_ZOOM_IN)
        zoom_in_button.connect('clicked', self.zoom_in_clicked)
        zoom_out_button = gtk.Button(stock=gtk.STOCK_ZOOM_OUT)
        zoom_out_button.connect('clicked', self.zoom_out_clicked)
        home_button = gtk.Button(stock=gtk.STOCK_HOME)
        home_button.connect('clicked', self.home_clicked)
        cache_button = gtk.Button('Cache')
        cache_button.connect('clicked', self.cache_clicked)

        vbox.pack_start(self.osm)
        hbox.pack_start(zoom_in_button)
        hbox.pack_start(zoom_out_button)
        hbox.pack_start(home_button)
        hbox.pack_start(cache_button)
        vbox.pack_start(hbox, False)
        vbox.pack_start(self.latlon_entry, False)

        gobject.timeout_add(500, self.print_tiles)
        

    def print_tiles(self):
        if self.osm.props.tiles_queued != 0:
            print self.osm.props.tiles_queued, 'tiles queued'
        return True

    def zoom_in_clicked(self, button):
        self.osm.set_zoom(self.osm.props.zoom + 1)
 
    def zoom_out_clicked(self, button):
        self.osm.set_zoom(self.osm.props.zoom - 1)

    def home_clicked(self, button):
        self.osm.set_mapcenter(-44.39, 171.25, 12)
 
    def cache_clicked(self, button):
        bbox = self.osm.get_bbox()
        self.osm.download_maps(
            *bbox,
            zoom_start=self.osm.props.zoom,
            zoom_end=self.osm.props.max_zoom
        )

    def map_clicked(self, osm, event):
        self.latlon_entry.set_text(
            'Map Centre: latitude %s longitude %s' % (
                self.osm.props.latitude,
                self.osm.props.longitude
            )
        )
 

if __name__ == "__main__":
    gtk.gdk.threads_init()

    u = UI()
    u.show_all()
    gtk.main()

