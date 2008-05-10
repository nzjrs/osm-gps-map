/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * osm-gps-map.c
 * Copyright (C) John Stowers 2008 <john.stowers@gmail.com>
 * 
 * osm-gps-map.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * osm-gps-map.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _OSM_GPS_MAP_H_
#define _OSM_GPS_MAP_H_

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

G_BEGIN_DECLS

#define OSM_TYPE_GPS_MAP             (osm_gps_map_get_type ())
#define OSM_GPS_MAP(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), OSM_TYPE_GPS_MAP, OsmGpsMap))
#define OSM_GPS_MAP_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), OSM_TYPE_GPS_MAP, OsmGpsMapClass))
#define OSM_IS_GPS_MAP(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), OSM_TYPE_GPS_MAP))
#define OSM_IS_GPS_MAP_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), OSM_TYPE_GPS_MAP))
#define OSM_GPS_MAP_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), OSM_TYPE_GPS_MAP, OsmGpsMapClass))

typedef struct _OsmGpsMapClass OsmGpsMapClass;
typedef struct _OsmGpsMap OsmGpsMap;

struct _OsmGpsMapClass
{
	GtkDrawingAreaClass parent_class;
};

struct _OsmGpsMap
{
	GtkDrawingArea parent_instance;
};

typedef struct {
	double lat1;
	double lon1;
	double lat2;
	double lon2;
} bbox_t;

typedef struct {
	int x;
	int y;
	int zoom;
//	repo_t *repo;
} tile_t;

GType osm_gps_map_get_type (void) G_GNUC_CONST;
void osm_gps_map_download_maps (OsmGpsMap *map, bbox_t bbox, int zoom_start, int zoom_end);
void osm_gps_map_download_tile (OsmGpsMap *map, int zoom, int x, int y);
bbox_t osm_gps_map_get_bbox (OsmGpsMap *map);
void osm_gps_map_map_redraw (OsmGpsMap *map);
void osm_gps_map_set_mapcenter (OsmGpsMap *map, float lat, float lon);
void osm_gps_map_print_track (OsmGpsMap *map, GList *trackpoint_list);
void osm_gps_map_paint_image (OsmGpsMap *map, float lat, float lon, GdkPixbuf *image, int w, int h);
tile_t osm_gps_map_get_tile (OsmGpsMap *map, int pixel_x, int pixel_y, int zoom);
void osm_gps_map_osd_speed (OsmGpsMap *map, float speed);
void osm_gps_map_draw_gps (OsmGpsMap *map, float lat, float lon);

G_END_DECLS

#endif /* _OSM_GPS_MAP_H_ */
