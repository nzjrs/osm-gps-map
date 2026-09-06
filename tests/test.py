#!/usr/bin/env python3
import os
import tempfile
import unittest
import cairo
import io

import gi
gi.require_version('OsmGpsMap', '1.0')
gi.require_foreign('cairo')

from gi.repository import OsmGpsMap
from gi.repository import Gdk, GdkPixbuf, GLib, GObject, Gtk

class TestOsmGpsMap(unittest.TestCase):
	def setUp(self):
		self.lat = 50
		self.lon = 13
		self.zoom = 15
		self.osm = OsmGpsMap.Map(user_agent="test/0.1")
		
	def test_map(self):
		test_window = Gtk.Window()
		test_window.set_title("OsmGpsMap")
		test_window.connect("destroy", Gtk.main_quit)
		test_window.show()
		test_window.set_size_request(640, 480)
		test_window.add(self.osm)
		test_window.show_all()
		
		self.osm.set_zoom(self.zoom)
		self.assertEqual(self.osm.get_property('zoom'), self.zoom)
		self.osm.zoom_in()
		self.assertEqual(self.osm.get_property('zoom'), self.zoom+1)
		self.osm.zoom_out()
		self.assertEqual(self.osm.get_property('zoom'), self.zoom)
		
		self.osm.set_center_and_zoom(self.lat, self.lon, self.zoom)
		self.assertEqual(self.osm.get_property("latitude"), self.lat)
		self.assertEqual(self.osm.get_property("longitude"), self.lon)
		
		self.lat += 1
		self.lon += 1
		self.osm.set_center(self.lat, self.lon)
		self.assertEqual(self.osm.get_property("latitude"), self.lat)
		self.assertEqual(self.osm.get_property("longitude"), self.lon)
		
		self.osm.set_property("map-source", OsmGpsMap.MapSource_t.OPENCYCLEMAP)
		self.osm.set_property("map-source", OsmGpsMap.MapSource_t.OPENSTREETMAP)
		
		self.osm.set_property("auto-center", False)
		self.osm.set_property("auto-center", True)

		self.osm.set_property("user-agent", "test/0.2")
		self.assertEqual(self.osm.get_property("user-agent"), "test/0.2")
		self.osm.set_property("user-agent", None)
		self.assertEqual(self.osm.get_property("user-agent"), None)
		
		self.osm.gps_add(self.lat, self.lon, heading=OsmGpsMap.MAP_INVALID);
		track = self.osm.gps_get_track()
		self.assertEqual(type(track), OsmGpsMap.MapTrack)
		self.osm.gps_clear()
		
	def test_layer(self):
		osd = OsmGpsMap.MapOsd(show_zoom=True, show_coordinates=False, show_scale=False, show_dpad=True, show_gps_in_dpad=True)
		self.osm.layer_add(osd)
		self.osm.layer_remove(osd)

	def test_custom_layer_draw(self):
		# MapLayer.do_draw gets the widget cairo_t; convert lat/lon in pixels.
		class Layer(GObject.GObject, OsmGpsMap.MapLayer):
			def __init__(self):
				GObject.GObject.__init__(self)
				self.drew = False
				self.xy = None

			def do_draw(self, gpsmap, cr):
				pt = OsmGpsMap.MapPoint.new_degrees(50.0, 13.0)
				x, y = gpsmap.convert_geographic_to_screen(pt)
				cr.set_source_rgba(1, 0, 0, 1)
				cr.arc(x, y, 4, 0, 6.28)
				cr.fill()
				self.xy = (x, y)
				self.drew = True

			def do_render(self, gpsmap):
				pass

			def do_busy(self):
				return False

			def do_button_press(self, gpsmap, event):
				return False

		GObject.type_register(Layer)
		layer = Layer()
		window = Gtk.Window()
		window.set_size_request(320, 240)
		window.add(self.osm)
		self.osm.layer_add(layer)
		self.osm.set_center_and_zoom(50.0, 13.0, 10)
		window.show_all()

		deadline = GLib.get_monotonic_time() + 500000
		while GLib.get_monotonic_time() < deadline and not layer.drew:
			Gtk.main_iteration_do(False)

		self.assertTrue(layer.drew)
		self.assertIsNotNone(layer.xy)
		alloc = self.osm.get_allocation()
		self.assertAlmostEqual(layer.xy[0], alloc.width / 2, delta=40)
		self.assertAlmostEqual(layer.xy[1], alloc.height / 2, delta=40)
		self.osm.layer_remove(layer)
		window.remove(self.osm)
		window.destroy()
		
	def test_image(self):
		size = 16
		drawable = cairo.ImageSurface(cairo.FORMAT_RGB24, size, size)
		ctx = cairo.Context(drawable)
		# transparent background
		ctx.set_source_rgba(1, 1, 1, 1)
		ctx.rectangle(0, 0, size, size)
		ctx.fill()
		ctx.stroke()
		# red arc
		ctx.set_source_rgb(1, 0, 0)
		ctx.arc(size/2, size/2, size/2-1, 0, 3.14*2)
		ctx.fill()
		ctx.stroke()
		
		# convert the cairo context to a GdkPixbuf
		buffer = io.BytesIO()
		drawable.write_to_png(buffer)
		loader = GdkPixbuf.PixbufLoader.new_with_type('png')
		loader.write(buffer.getvalue())
		buffer.close()
		loader.close()
		pixbuf = loader.get_pixbuf()
		image = pixbuf.add_alpha(True , 255, 255, 255)
		
		for x in range(0, 5):
			pointer = self.osm.image_add(self.lat+x, self.lon+x, image)
		
		self.osm.image_remove(pointer)
		self.osm.image_remove_all()
		
	def test_track(self):
		track = OsmGpsMap.MapTrack()
		self.osm.track_add(track)
		
		color = Gdk.RGBA(1, 0, 1, 0)
		track.set_property('color', color)
		color2 = track.get_color()
		self.assertEqual(color, color2)
		
		points = []
		for x in range(0, 5):
			point = OsmGpsMap.MapPoint.new_degrees(self.lat+x, self.lon+x)
			track.add_point(point)
			points.append(point)
		
		self.assertEqual(track.n_points(), 5)
		self.assertEqual(track.get_length(), 522318.175858659)
		
		track.remove_point(3)
		self.assertEqual(track.n_points(), 4)
		
		self.assertEqual(type(track.get_point(3)), OsmGpsMap.MapPoint)
		
		returned_points = track.get_points()
		self.assertEqual(len(returned_points), 4)
		
		self.osm.track_remove(track)

	def test_insert_point(self):
		# Issue #45: check insert_point does not double-free.
		track = OsmGpsMap.MapTrack()
		point = OsmGpsMap.MapPoint.new_degrees(self.lat, self.lon)
		track.insert_point(point, 0)
		self.assertEqual(track.n_points(), 1)

	def test_convert_screen_to_geographic(self):
		# GI returns the MapPoint; do not pass one in.
		pt = self.osm.convert_screen_to_geographic(0, 0)
		self.assertEqual(type(pt), OsmGpsMap.MapPoint)
		lat, lon = pt.get_degrees()
		self.assertTrue(-90.0 <= lat <= 90.0)
		self.assertTrue(-180.0 <= lon <= 180.0)

	def test_zoom_fit_bbox_point(self):
		# Degenerate bbox (one geotag). Must not crash; zoom clamps to max.
		self.osm.zoom_fit_bbox(self.lat, self.lat, self.lon, self.lon)
		self.assertEqual(self.osm.get_property('zoom'),
				 self.osm.get_property('max-zoom'))

	def test_negative_map_origin_tiles_align_with_overlay(self):
		# Issue #119: C / truncates toward 0. Negative map-x/map-y must floor so
		# tiles stay locked to overlays. Zoom 1, 800x800 window, center 0,0
		# makes both origins negative. Red cached tiles vs white outside the world.
		cache_dir = tempfile.TemporaryDirectory(prefix="osm-gps-map-tiles-")
		self.addCleanup(cache_dir.cleanup)
		cache = cache_dir.name
		tile = cairo.ImageSurface(cairo.FORMAT_RGB24, 256, 256)
		cr = cairo.Context(tile)
		cr.set_source_rgb(1, 0, 0)
		cr.paint()
		for x in (0, 1):
			for y in (0, 1):
				path = os.path.join(cache, "1", str(x), "%d.png" % y)
				os.makedirs(os.path.dirname(path), exist_ok=True)
				tile.write_to_png(path)

		osm = OsmGpsMap.Map(user_agent="test/0.1",
				    tile_cache=cache,
				    auto_download=False)
		window = Gtk.OffscreenWindow()
		self.addCleanup(window.destroy)
		window.set_default_size(800, 800)
		window.add(osm)
		self.addCleanup(window.remove, osm)
		window.show_all()
		osm.set_center_and_zoom(0.0, 0.0, 1)

		deadline = GLib.get_monotonic_time() + 1000000
		r = g = b = None
		while GLib.get_monotonic_time() < deadline:
			Gtk.main_iteration_do(False)
			if osm.get_property("map-x") >= 0 or osm.get_property("map-y") >= 0:
				continue
			pixbuf = window.get_pixbuf()
			if pixbuf is None or pixbuf.get_width() < 800 or pixbuf.get_height() < 800:
				continue
			pt = OsmGpsMap.MapPoint.new_degrees(0.0, 0.0)
			sx, sy = osm.convert_geographic_to_screen(pt)
			pixels = pixbuf.get_pixels()
			nch = pixbuf.get_n_channels()
			row = pixbuf.get_rowstride()
			i = int(sy) * row + int(sx) * nch
			r, g, b = pixels[i], pixels[i + 1], pixels[i + 2]
			if r + g + b > 0:
				break

		self.assertLess(osm.get_property("map-x"), 0)
		self.assertNotEqual(osm.get_property("map-x") % 256, 0)
		self.assertLess(osm.get_property("map-y"), 0)
		self.assertNotEqual(osm.get_property("map-y") % 256, 0)
		self.assertIsNotNone(r)
		self.assertGreater(r, 200)
		self.assertLess(g, 50)
		self.assertLess(b, 50)

if __name__ == "__main__":
	unittest.main()
