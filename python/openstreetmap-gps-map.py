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
import math
import sys
import os.path
import gtk.gdk
import gobject

#Try static lib first
mydir = os.path.dirname(os.path.abspath(__file__))
libdir = os.path.join(mydir, ".libs")
sys.path.insert(0, libdir)

import osmgpsmap
print "using library: %s" % osmgpsmap.__file__
 
class UI(gtk.Window):
    def __init__(self):
        gtk.Window.__init__(self, gtk.WINDOW_TOPLEVEL)

        self.set_default_size(500, 500)
        self.connect('destroy', lambda x: gtk.main_quit())
        self.set_title('OpenStreetMap GPS Mapper')

        self.vbox = gtk.VBox(False, 0)
        self.add(self.vbox)

        self.osm = osmgpsmap.GpsMap(
            tile_cache=osmgpsmap.get_default_cache_directory(),
        )
        self.osm.connect('button_release_event', self.map_clicked)

        #connect keyboard shortcuts
        self.osm.set_keyboard_shortcut(osmgpsmap.KEY_FULLSCREEN, gtk.gdk.keyval_from_name("F11"))
        self.osm.set_keyboard_shortcut(osmgpsmap.KEY_UP, gtk.gdk.keyval_from_name("Up"))
        self.osm.set_keyboard_shortcut(osmgpsmap.KEY_DOWN, gtk.gdk.keyval_from_name("Down"))
        self.osm.set_keyboard_shortcut(osmgpsmap.KEY_LEFT, gtk.gdk.keyval_from_name("Left"))
        self.osm.set_keyboard_shortcut(osmgpsmap.KEY_RIGHT, gtk.gdk.keyval_from_name("Right"))

        self.latlon_entry = gtk.Entry()

        zoom_in_button = gtk.Button(stock=gtk.STOCK_ZOOM_IN)
        zoom_in_button.connect('clicked', self.zoom_in_clicked)
        zoom_out_button = gtk.Button(stock=gtk.STOCK_ZOOM_OUT)
        zoom_out_button.connect('clicked', self.zoom_out_clicked)
        home_button = gtk.Button(stock=gtk.STOCK_HOME)
        home_button.connect('clicked', self.home_clicked)
        cache_button = gtk.Button('Cache')
        cache_button.connect('clicked', self.cache_clicked)

        self.vbox.pack_start(self.osm)
        hbox = gtk.HBox(False, 0)
        hbox.pack_start(zoom_in_button)
        hbox.pack_start(zoom_out_button)
        hbox.pack_start(home_button)
        hbox.pack_start(cache_button)

        #add ability to test custom map URIs
        ex = gtk.Expander("<b>Map Repository URI</b>")
        ex.props.use_markup = True
        vb = gtk.VBox()
        self.repouri_entry = gtk.Entry()
        self.repouri_entry.set_text(self.osm.props.repo_uri)
        self.image_format_entry = gtk.Entry()
        self.image_format_entry.set_text(self.osm.props.image_format)
        gobtn = gtk.Button("Load Map URI")
        gobtn.connect("clicked", self.load_map_clicked)
        exp = gtk.Label(
"""
Enter an repository URL to fetch map tiles from in the box below. Special metacharacters may be included in this url

<i>Metacharacters:</i>
\t#X\tMax X location
\t#Y\tMax Y location
\t#Z\tMap zoom (0 = min zoom, fully zoomed out)
\t#S\tInverse zoom (max-zoom - #Z)
\t#Q\tQuadtree encoded tile (qrts)
\t#W\tQuadtree encoded tile (1234)
\t#U\tEncoding not implemeted
\t#R\tRandom integer, 0-4""")
        exp.props.xalign = 0
        exp.props.use_markup = True
        exp.props.wrap = True

        ex.add(vb)
        vb.pack_start(exp, False)

        hb = gtk.HBox()
        hb.pack_start(gtk.Label("URI: "), False)
        hb.pack_start(self.repouri_entry, True)
        vb.pack_start(hb, False)

        hb = gtk.HBox()
        hb.pack_start(gtk.Label("Image Format: "), False)
        hb.pack_start(self.image_format_entry, True)
        vb.pack_start(hb, False)

        vb.pack_start(gobtn, False)

        self.vbox.pack_end(ex, False)
        self.vbox.pack_end(self.latlon_entry, False)
        self.vbox.pack_end(hbox, False)

        gobject.timeout_add(500, self.print_tiles)

    def load_map_clicked(self, button):
        uri = self.repouri_entry.get_text()
        format = self.image_format_entry.get_text()
        if uri and format:
            if self.osm:
                #remove old map
                self.vbox.remove(self.osm)

            try:
                self.osm = osmgpsmap.GpsMap(
                    tile_cache=osmgpsmap.get_default_cache_directory(),
                    repo_uri=uri,
                    image_format=format
                )
                self.osm.connect('button_release_event', self.map_clicked)
            except Exception, e:
                print "ERROR:", e
                self.osm = osm.GpsMap()

            self.vbox.pack_start(self.osm, True)
            self.osm.connect('button_release_event', self.map_clicked)
            self.osm.show()

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
        if event.button == 1:
            self.latlon_entry.set_text(
                'Map Centre: latitude %s longitude %s' % (
                    self.osm.props.latitude,
                    self.osm.props.longitude
                )
            )
        elif event.button == 2:
            rlat, rlon = self.osm.get_co_ordinates(event.x, event.y)
            self.osm.draw_gps(
                    math.degrees(rlat),
                    math.degrees(rlon),
                    0.0);
 

if __name__ == "__main__":
    gtk.gdk.threads_init()

    u = UI()
    u.show_all()
    gtk.main()

