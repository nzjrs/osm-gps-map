#!/usr/bin/seed

const Gtk = imports.gi.Gtk;
const Osm = imports.gi.OsmGpsMap;

Gtk.init(0, null);

var win = new Gtk.Window({ type: Gtk.WindowType.TOPLEVEL });
win.set_border_width(10);
win.set_default_size(400,400);

// Fuck you GNOME
// GJS makes me do this
//win.connect("delete-event", Gtk.main_quit);
// Seed makes me do this
win.signal.delete_event.connect(Gtk.main_quit);

var map = new Osm.OsmGpsMap()
var osd = new Osm.Osd()

map.layer_add(osd)
win.add(map);

win.show_all();

Gtk.main();


