#!/usr/bin/env python3
import unittest
import cairo
import io

import gi
gi.require_version('OsmGpsMap', '1.0')

from gi.repository import OsmGpsMap
from gi.repository import Gdk, GdkPixbuf, Gtk

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
		pixbuf = loader.get_pixbuf()
		loader.close()
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

if __name__ == "__main__":
	unittest.main()
