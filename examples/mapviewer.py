#!/usr/bin/python

"""
Copyright (C) Hadley Rich 2008 <hads@nice.net.nz>
based on main.c - with thanks to John Stowers

This is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License
as published by the Free Software Foundation; version 2.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, see <http://www.gnu.org/licenses/>.
"""

import sys
import os.path
import random
from math import pi
from gi.repository import Gtk
from gi.repository import Gdk
from gi.repository import GdkPixbuf
from gi.repository import GObject

GObject.threads_init()
Gdk.threads_init()

from gi.repository import OsmGpsMap as osmgpsmap
print "using library: %s (version %s)" % (osmgpsmap.__file__, osmgpsmap._version)

assert osmgpsmap._version == "1.0"

class DummyMapNoGpsPoint(osmgpsmap.Map):
    def do_draw_gps_point(self, drawable):
        pass
GObject.type_register(DummyMapNoGpsPoint)

class DummyLayer(GObject.GObject, osmgpsmap.MapLayer):
    def __init__(self):
        GObject.GObject.__init__(self)

    def do_draw(self, gpsmap, gdkdrawable):
        pass

    def do_render(self, gpsmap):
        pass

    def do_busy(self):
        return False

    def do_button_press(self, gpsmap, gdkeventbutton):
        return False
GObject.type_register(DummyLayer)

class UI(Gtk.Window):
    def __init__(self):
        Gtk.Window.__init__(self, type=Gtk.WindowType.TOPLEVEL)

        self.set_default_size(500, 500)
        self.connect('destroy', lambda x: Gtk.main_quit())
        self.set_title('OpenStreetMap GPS Mapper')

        self.vbox = Gtk.VBox(False, 0)
        self.add(self.vbox)

        if 0:
            self.osm = DummyMapNoGpsPoint()
        else:
            self.osm = osmgpsmap.Map(user_agent="mapviewer.py/%s" % osmgpsmap._version)
        self.osm.layer_add(
                    osmgpsmap.MapOsd(
                        show_dpad=True,
                        show_zoom=True,
                        show_crosshair=True)
        )
        self.osm.layer_add(
                    DummyLayer()
        )

        self.last_image = None

        self.osm.connect('button_press_event', self.on_button_press)
        self.osm.connect('button_release_event', self.on_button_release)

        #connect keyboard shortcuts
        self.osm.set_keyboard_shortcut(osmgpsmap.MapKey_t.FULLSCREEN, Gdk.keyval_from_name("F11"))
        self.osm.set_keyboard_shortcut(osmgpsmap.MapKey_t.UP, Gdk.keyval_from_name("Up"))
        self.osm.set_keyboard_shortcut(osmgpsmap.MapKey_t.DOWN, Gdk.keyval_from_name("Down"))
        self.osm.set_keyboard_shortcut(osmgpsmap.MapKey_t.LEFT, Gdk.keyval_from_name("Left"))
        self.osm.set_keyboard_shortcut(osmgpsmap.MapKey_t.RIGHT, Gdk.keyval_from_name("Right"))

        #connect to tooltip
        self.osm.props.has_tooltip = True
        self.osm.connect("query-tooltip", self.on_query_tooltip)

        self.latlon_entry = Gtk.Entry()

        zoom_in_button = Gtk.Button(stock=Gtk.STOCK_ZOOM_IN)
        zoom_in_button.connect('clicked', self.zoom_in_clicked)
        zoom_out_button = Gtk.Button(stock=Gtk.STOCK_ZOOM_OUT)
        zoom_out_button.connect('clicked', self.zoom_out_clicked)
        home_button = Gtk.Button(stock=Gtk.STOCK_HOME)
        home_button.connect('clicked', self.home_clicked)
        cache_button = Gtk.Button('Cache')
        cache_button.connect('clicked', self.cache_clicked)

        self.vbox.pack_start(self.osm, True, True, 0)
        hbox = Gtk.HBox(False, 0)
        hbox.pack_start(zoom_in_button, False, True, 0)
        hbox.pack_start(zoom_out_button, False, True, 0)
        hbox.pack_start(home_button, False, True, 0)
        hbox.pack_start(cache_button, False, True, 0)

        #add ability to test custom map URIs
        ex = Gtk.Expander(label="<b>Map Repository URI</b>")
        ex.props.use_markup = True
        vb = Gtk.VBox()
        self.repouri_entry = Gtk.Entry()
        self.repouri_entry.set_text(self.osm.props.repo_uri)
        self.image_format_entry = Gtk.Entry()
        self.image_format_entry.set_text(self.osm.props.image_format)

        lbl = Gtk.Label(
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
        lbl.props.xalign = 0
        lbl.props.use_markup = True
        lbl.props.wrap = True

        ex.add(vb)
        vb.pack_start(lbl, False, True, 0)

        hb = Gtk.HBox()
        hb.pack_start(Gtk.Label("URI: "), False, True, 0)
        hb.pack_start(self.repouri_entry, True, True, 0)
        vb.pack_start(hb, False, True, 0)

        hb = Gtk.HBox()
        hb.pack_start(Gtk.Label("Image Format: "), False, True, 0)
        hb.pack_start(self.image_format_entry, True, True, 0)
        vb.pack_start(hb, False, True, 0)

        gobtn = Gtk.Button("Load Map URI")
        gobtn.connect("clicked", self.load_map_clicked)
        vb.pack_start(gobtn, False, True, 0)

        self.show_tooltips = False
        cb = Gtk.CheckButton("Show Location in Tooltips")
        cb.props.active = self.show_tooltips
        cb.connect("toggled", self.on_show_tooltips_toggled)
        self.vbox.pack_end(cb, False, True, 0)

        cb = Gtk.CheckButton("Disable Cache")
        cb.props.active = False
        cb.connect("toggled", self.disable_cache_toggled)
        self.vbox.pack_end(cb, False, True, 0)

        self.vbox.pack_end(ex, False, True, 0)
        self.vbox.pack_end(self.latlon_entry, False, True, 0)
        self.vbox.pack_end(hbox, False, True, 0)

        GObject.timeout_add(500, self.print_tiles)

    def disable_cache_toggled(self, btn):
        if btn.props.active:
            self.osm.props.tile_cache = osmgpsmap.MAP_CACHE_DISABLED
        else:
            self.osm.props.tile_cache = osmgpsmap.MAP_CACHE_AUTO

    def on_show_tooltips_toggled(self, btn):
        self.show_tooltips = btn.props.active

    def load_map_clicked(self, button):
        uri = self.repouri_entry.get_text()
        format = self.image_format_entry.get_text()
        if uri and format:
            if self.osm:
                #remove old map
                self.vbox.remove(self.osm)
            try:
                self.osm = osmgpsmap.Map(
                    repo_uri=uri,
                    image_format=format
                )
            except Exception, e:
                print "ERROR:", e
                self.osm = osm.Map()

            self.vbox.pack_start(self.osm, True, True, 0)
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
        self.osm.set_center_and_zoom(-44.39, 171.25, 12)

    def on_query_tooltip(self, widget, x, y, keyboard_tip, tooltip, data=None):
        if keyboard_tip:
            return False

        if self.show_tooltips:
            p = osmgpsmap.point_new_degrees(0.0, 0.0)
            self.osm.convert_screen_to_geographic(x, y, p)
            lat,lon = p.get_degrees()
            tooltip.set_markup("%+.4f, %+.4f" % (lat, lon ))
            return True

        return False
 
    def cache_clicked(self, button):
        bbox = self.osm.get_bbox()
        self.osm.download_maps(
            *bbox,
            zoom_start=self.osm.props.zoom,
            zoom_end=self.osm.props.max_zoom
        )

    def on_button_release(self, osm, event):
        self.latlon_entry.set_text(
            'Map Centre: latitude %s longitude %s' % (
                self.osm.props.latitude,
                self.osm.props.longitude
            )
        )

    def on_button_press(self, osm, event):
        state = event.get_state()
        lat,lon = self.osm.get_event_location(event).get_degrees()

        left    = event.button == 1 and state == 0
        middle  = event.button == 2 or (event.button == 1 and state & Gdk.ModifierType.SHIFT_MASK)
        right   = event.button == 3 or (event.button == 1 and state & Gdk.ModifierType.CONTROL_MASK)

        #work around binding bug with invalid variable name
        GDK_2BUTTON_PRESS = getattr(Gdk.EventType, "2BUTTON_PRESS")
        GDK_3BUTTON_PRESS = getattr(Gdk.EventType, "3BUTTON_PRESS")

        if event.type == GDK_3BUTTON_PRESS:
            if middle:
                if self.last_image is not None:
                    self.osm.image_remove(self.last_image)
                    self.last_image = None
        elif event.type == GDK_2BUTTON_PRESS:
            if left:
                self.osm.gps_add(lat, lon, heading=random.random()*360)
            if middle:
                pb = GdkPixbuf.Pixbuf.new_from_file_at_size ("poi.png", 24,24)
                self.last_image = self.osm.image_add(lat,lon,pb)
            if right:
                pass


if __name__ == "__main__":
    u = UI()
    u.show_all()
    if os.name == "nt": Gdk.threads_enter()
    Gtk.main()
    if os.name == "nt": Gdk.threads_leave()

