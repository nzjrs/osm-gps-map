/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*- */
/* vim:set et sw=4 ts=4 */
/*
 * Copyright (C) 2013 John Stowers <john.stowers@gmail.com>
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

#ifndef _OSM_GPS_MAP_IMAGE_H
#define _OSM_GPS_MAP_IMAGE_H

#include <glib-object.h>
#include <gdk/gdk.h>
#include <cairo.h>

#include "osm-gps-map-point.h"

G_BEGIN_DECLS

#define OSM_TYPE_GPS_MAP_IMAGE              osm_gps_map_image_get_type()
#define OSM_GPS_MAP_IMAGE(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), OSM_TYPE_GPS_MAP_IMAGE, OsmGpsMapImage))
#define OSM_GPS_MAP_IMAGE_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), OSM_TYPE_GPS_MAP_IMAGE, OsmGpsMapImageClass))
#define OSM_GPS_MAP_IS_IMAGE(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), OSM_TYPE_GPS_MAP_IMAGE))
#define OSM_GPS_MAP_IS_IMAGE_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), OSM_TYPE_GPS_MAP_IMAGE))
#define OSM_GPS_MAP_IMAGE_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), OSM_TYPE_GPS_MAP_IMAGE, OsmGpsMapImageClass))

typedef struct _OsmGpsMapImage OsmGpsMapImage;
typedef struct _OsmGpsMapImageClass OsmGpsMapImageClass;
typedef struct _OsmGpsMapImagePrivate OsmGpsMapImagePrivate;

struct _OsmGpsMapImage
{
    GObject parent;

    OsmGpsMapImagePrivate *priv;
};

struct _OsmGpsMapImageClass
{
    GObjectClass parent_class;
};

/**
 * osm_gps_map_image_get_type:
 *
 * Get image type
 *
 * Return value: (element-type GType): The type of the image
 * Since: 0.7.0
 **/
GType osm_gps_map_image_get_type (void) G_GNUC_CONST;

/**
 * osm_gps_map_image_new:
 *
 * Create new image
 *
 * Returns: (transfer full): New image
 * Since: 0.7.0
 **/
OsmGpsMapImage *osm_gps_map_image_new (void);
/**
 * osm_gps_map_image_draw:
 * @object: a #OsmGpsMapImage
 * @cr: cairo context
 * @rect: (inout): bounding rectangle
 *
 * Draw image to given cairo context
 *
 * Since: 0.7.0
 **/
void            osm_gps_map_image_draw (OsmGpsMapImage *object, cairo_t *cr, GdkRectangle *rect);
/**
 * osm_gps_map_image_get_point:
 * @object: a #OsmGpsMapImage
 *
 * Get image location point
 *
 * Returns: (transfer none): location point
 * Since: 0.7.0
 **/
const OsmGpsMapPoint *osm_gps_map_image_get_point(OsmGpsMapImage *object);
/**
 * osm_gps_map_image_get_zorder:
 * @object: a #OsmGpsMapImage
 *
 * Get image z-order
 *
 * Returns: z-order
 * Since: 1.0.0
 **/
gint osm_gps_map_image_get_zorder(OsmGpsMapImage *object);
/**
 * osm_gps_map_image_get_rotation:
 * @object: a #OsmGpsMapImage
 *
 * Get image rotation
 *
 * Returns: rotation
 * Since: 1.1.0
 **/
float osm_gps_map_image_get_rotation(OsmGpsMapImage* object);
/**
 * osm_gps_map_image_set_rotation:
 * @object: a #OsmGpsMapImage
 * @rot: image rotation in degrees
 *
 * Set image rotation
 * Since: 1.1.0
 **/
void osm_gps_map_image_set_rotation(OsmGpsMapImage* object, float rot);

G_END_DECLS

#endif /* _OSM_GPS_MAP_IMAGE_H */
