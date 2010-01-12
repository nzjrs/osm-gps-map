/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*- */
/* vim:set et sw=4 ts=4 cino=t0,(0: */
/*
 * osm-gps-map.h
 * Copyright (C) Marcus Bauer 2008 <marcus.bauer@gmail.com>
 * Copyright (C) John Stowers 2009 <john.stowers@gmail.com>
 * Copyright (C) Till Harbaum 2009 <till@harbaum.org>
 *
 * Contributions by
 * Everaldo Canuto 2009 <everaldo.canuto@gmail.com>
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
typedef struct _OsmGpsMapPrivate OsmGpsMapPrivate;

struct _OsmGpsMapClass
{
    GtkDrawingAreaClass parent_class;
};

struct _OsmGpsMap
{
    GtkDrawingArea parent_instance;
    OsmGpsMapPrivate *priv;
};

typedef struct {
    float rlat;
    float rlon;
} coord_t;

typedef enum {
    OSM_GPS_MAP_SOURCE_NULL,
    OSM_GPS_MAP_SOURCE_OPENSTREETMAP,
    OSM_GPS_MAP_SOURCE_OPENSTREETMAP_RENDERER,
    OSM_GPS_MAP_SOURCE_OPENAERIALMAP,
    OSM_GPS_MAP_SOURCE_MAPS_FOR_FREE,
    OSM_GPS_MAP_SOURCE_GOOGLE_STREET,
    OSM_GPS_MAP_SOURCE_GOOGLE_SATELLITE,
    OSM_GPS_MAP_SOURCE_GOOGLE_HYBRID,
    OSM_GPS_MAP_SOURCE_VIRTUAL_EARTH_STREET,
    OSM_GPS_MAP_SOURCE_VIRTUAL_EARTH_SATELLITE,
    OSM_GPS_MAP_SOURCE_VIRTUAL_EARTH_HYBRID,
    OSM_GPS_MAP_SOURCE_YAHOO_STREET,
    OSM_GPS_MAP_SOURCE_YAHOO_SATELLITE,
    OSM_GPS_MAP_SOURCE_YAHOO_HYBRID
} OsmGpsMapSource_t;
#define OSM_GPS_MAP_SOURCE_LAST     OSM_GPS_MAP_SOURCE_YAHOO_HYBRID

typedef enum {
    OSM_GPS_MAP_KEY_FULLSCREEN,
    OSM_GPS_MAP_KEY_ZOOMIN,
    OSM_GPS_MAP_KEY_ZOOMOUT,
    OSM_GPS_MAP_KEY_UP,
    OSM_GPS_MAP_KEY_DOWN,
    OSM_GPS_MAP_KEY_LEFT,
    OSM_GPS_MAP_KEY_RIGHT,
    OSM_GPS_MAP_KEY_MAX
} OsmGpsMapKey_t;

#define OSM_GPS_MAP_INVALID         (0.0/0.0)
#define OSM_GPS_MAP_CACHE_DISABLED  "none://"
#define OSM_GPS_MAP_CACHE_AUTO      "auto://"

GType       osm_gps_map_get_type                    (void) G_GNUC_CONST;

GtkWidget*  osm_gps_map_new                         (void);

char*       osm_gps_map_get_default_cache_directory (void);

const char* osm_gps_map_source_get_friendly_name    (OsmGpsMapSource_t source);
const char* osm_gps_map_source_get_repo_uri         (OsmGpsMapSource_t source);
const char* osm_gps_map_source_get_image_format     (OsmGpsMapSource_t source);
int         osm_gps_map_source_get_min_zoom         (OsmGpsMapSource_t source);
int         osm_gps_map_source_get_max_zoom         (OsmGpsMapSource_t source);

void        osm_gps_map_download_maps               (OsmGpsMap *map, coord_t *pt1, coord_t *pt2, int zoom_start, int zoom_end);
void        osm_gps_map_get_bbox                    (OsmGpsMap *map, coord_t *pt1, coord_t *pt2);
void        osm_gps_map_set_mapcenter               (OsmGpsMap *map, float latitude, float longitude, int zoom);
void        osm_gps_map_set_center                  (OsmGpsMap *map, float latitude, float longitude);
int         osm_gps_map_set_zoom                    (OsmGpsMap *map, int zoom);
void        osm_gps_map_add_track                   (OsmGpsMap *map, GSList *track);
void        osm_gps_map_replace_track               (OsmGpsMap *map, GSList *old_track, GSList *new_track);
void        osm_gps_map_clear_tracks                (OsmGpsMap *map);
void        osm_gps_map_add_image                   (OsmGpsMap *map, float latitude, float longitude, GdkPixbuf *image);
gboolean    osm_gps_map_remove_image                (OsmGpsMap *map, GdkPixbuf *image);
void        osm_gps_map_clear_images                (OsmGpsMap *map);
void        osm_gps_map_draw_gps                    (OsmGpsMap *map, float latitude, float longitude, float heading);
void        osm_gps_map_clear_gps                   (OsmGpsMap *map);
coord_t     osm_gps_map_get_co_ordinates            (OsmGpsMap *map, int pixel_x, int pixel_y);
void        osm_gps_map_screen_to_geographic        (OsmGpsMap *map, gint pixel_x, gint pixel_y, gfloat *latitude, gfloat *longitude);
void        osm_gps_map_geographic_to_screen        (OsmGpsMap *map, gfloat latitude, gfloat longitude, gint *pixel_x, gint *pixel_y);
void        osm_gps_map_scroll                      (OsmGpsMap *map, gint dx, gint dy);
float       osm_gps_map_get_scale                   (OsmGpsMap *map);
void        osm_gps_map_set_keyboard_shortcut       (OsmGpsMap *map, OsmGpsMapKey_t key, guint keyval);

G_END_DECLS

#endif /* _OSM_GPS_MAP_H_ */
