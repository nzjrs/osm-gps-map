/*
 * Copyright (C) 2013 Samuel Cowen <samuel.cowen@camelsoftware.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */


#ifndef _OSM_GPS_MAP_POLYGON_H
#define _OSM_GPS_MAP_POLYGON_H

#include <glib.h>
#include <glib-object.h>
#include <gdk/gdk.h>

G_BEGIN_DECLS

#include "osm-gps-map-track.h"

#define OSM_TYPE_GPS_MAP_POLYGON              osm_gps_map_polygon_get_type()
#define OSM_GPS_MAP_POLYGON(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), OSM_TYPE_GPS_MAP_POLYGON, OsmGpsMapPolygon))
#define OSM_GPS_MAP_POLYGON_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), OSM_TYPE_GPS_MAP_POLYGON, OsmGpsMapPolygonClass))
#define OSM_IS_GPS_MAP_POLYGON(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), OSM_TYPE_GPS_MAP_POLYGON))
#define OSM_IS_GPS_MAP_POLYGON_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), OSM_TYPE_GPS_MAP_POLYGON))
#define OSM_GPS_MAP_POLYGON_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), OSM_TYPE_GPS_MAP_POLYGON, OsmGpsMapPolygonClass))

typedef struct _OsmGpsMapPolygon OsmGpsMapPolygon;
typedef struct _OsmGpsMapPolygonClass OsmGpsMapPolygonClass;
typedef struct _OsmGpsMapPolygonPrivate OsmGpsMapPolygonPrivate;

struct _OsmGpsMapPolygon
{
    GObject parent;

    OsmGpsMapPolygonPrivate *priv;
};

struct _OsmGpsMapPolygonClass
{
    GObjectClass parent_class;
};

GType osm_gps_map_polygon_get_type (void) G_GNUC_CONST;

OsmGpsMapPolygon*		osm_gps_map_polygon_new           (void);

/**
 * osm_gps_map_polygon_get_track:
 * @poly: a #OsmGpsMapPolygon
 *
 * Returns: (transfer none): The #OsmGpsMapTrack of the polygon
 **/
OsmGpsMapTrack*			osm_gps_map_polygon_get_track(OsmGpsMapPolygon* poly);

G_END_DECLS

#endif /* _OSM_GPS_MAP_POLYGON_H */
