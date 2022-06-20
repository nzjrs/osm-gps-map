/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*- */
/* vim:set et sw=4 ts=4 cino=t0,(0: */
/*
 * Copyright (C) Marcus Bauer 2008 <marcus.bauer@gmail.com>
 * Copyright (C) John Stowers 2013 <john.stowers@gmail.com>
 * Copyright (C) Till Harbaum 2009 <till@harbaum.org>
 *
 * Contributions by
 * Everaldo Canuto 2009 <everaldo.canuto@gmail.com>
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

#ifndef _OSM_GPS_MAP_LAYER_H_
#define _OSM_GPS_MAP_LAYER_H_

#include <glib.h>
#include <glib-object.h>
#include <gdk/gdk.h>
#include <cairo.h>

G_BEGIN_DECLS

#define OSM_TYPE_GPS_MAP_LAYER                  (osm_gps_map_layer_get_type ())
#define OSM_GPS_MAP_LAYER(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), OSM_TYPE_GPS_MAP_LAYER, OsmGpsMapLayer))
#define OSM_GPS_MAP_IS_LAYER(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), OSM_TYPE_GPS_MAP_LAYER))
#define OSM_GPS_MAP_LAYER_GET_INTERFACE(inst)   (G_TYPE_INSTANCE_GET_INTERFACE ((inst), OSM_TYPE_GPS_MAP_LAYER, OsmGpsMapLayerIface))

typedef struct _OsmGpsMapLayer          OsmGpsMapLayer;             /* dummy object */
typedef struct _OsmGpsMapLayerIface     OsmGpsMapLayerIface;

#include "osm-gps-map-widget.h"

struct _OsmGpsMapLayerIface {
    GTypeInterface parent;

    void (*render) (OsmGpsMapLayer *self, OsmGpsMap *map);
    void (*draw) (OsmGpsMapLayer *self, OsmGpsMap *map, cairo_t *cr);
    gboolean (*busy) (OsmGpsMapLayer *self);
    gboolean (*button_press) (OsmGpsMapLayer *self, OsmGpsMap *map, GtkGestureSingle *event, gint n_press, gdouble x, gdouble y, gpointer user_data);
};

/**
 * osm_gps_map_layer_get_type:
 *
 * Get layer type
 *
 * Return value: (element-type GType): The type of the layer
 * Since: 0.6.0
 **/
GType osm_gps_map_layer_get_type (void);

/**
 * osm_gps_map_layer_render:
 * @self: (in): a #OsmGpsMapLayer object
 * @map: (in): a #OsmGpsMap widget
 *
 * Render layer on map
 *
 * Since: 0.6.0
 **/
void        osm_gps_map_layer_render            (OsmGpsMapLayer *self, OsmGpsMap *map);

/**
 * osm_gps_map_layer_draw:
 * @self: (in): a #OsmGpsMapLayer object
 * @map: (in): a #OsmGpsMap widget
 * @cr: (in): a cairo context to draw to
 *
 * Draw layer on map
 *
 * Since: 0.6.0
 **/
void        osm_gps_map_layer_draw              (OsmGpsMapLayer *self, OsmGpsMap *map, cairo_t *cr);

/**
 * osm_gps_map_layer_busy:
 * @self: (in): a #OsmGpsMapLayer object
 *
 * Check whether layer is busy (eg drawing an animation)
 *
 * Returns: layer busy state
 * Since: 0.6.0
 **/
gboolean    osm_gps_map_layer_busy              (OsmGpsMapLayer *self);

/**
 * osm_gps_map_layer_button_press:
 * @self: (in): a #OsmGpsMapLayer object
 * @map: (in): a #OsmGpsMap widget
 * @event: (in): a GtkGestureSingle event
 * @n_press: (in): How many touch/button presses happened with this one.
 * @x: (in): The X coordinate, in widget allocation coordinates.
 * @y: (in): The Y coordinate, in widget allocation coordinates.
 * @user_data: (in): pointer to user data
 *
 * Handle button event
 *
 * Returns: whether even had been handled
 * Since: 0.6.0
 **/
gboolean    osm_gps_map_layer_button_press      (OsmGpsMapLayer *self, OsmGpsMap *map, GtkGestureSingle *event, gint n_press, gdouble x, gdouble y, gpointer user_data);

G_END_DECLS

#endif /* _OSM_GPS_MAP_LAYER_H_ */
