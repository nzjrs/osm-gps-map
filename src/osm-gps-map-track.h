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

#ifndef _OSM_GPS_MAP_TRACK_H
#define _OSM_GPS_MAP_TRACK_H

#include <glib.h>
#include <glib-object.h>
#include <gdk/gdk.h>

#include "osm-gps-map-point.h"

G_BEGIN_DECLS

#define OSM_TYPE_GPS_MAP_TRACK              osm_gps_map_track_get_type()
#define OSM_GPS_MAP_TRACK(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), OSM_TYPE_GPS_MAP_TRACK, OsmGpsMapTrack))
#define OSM_GPS_MAP_TRACK_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), OSM_TYPE_GPS_MAP_TRACK, OsmGpsMapTrackClass))
#define OSM_GPS_MAP_IS_TRACK(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), OSM_TYPE_GPS_MAP_TRACK))
#define OSM_GPS_MAP_IS_TRACK_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), OSM_TYPE_GPS_MAP_TRACK))
#define OSM_GPS_MAP_TRACK_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), OSM_TYPE_GPS_MAP_TRACK, OsmGpsMapTrackClass))

typedef struct _OsmGpsMapTrack OsmGpsMapTrack;
typedef struct _OsmGpsMapTrackClass OsmGpsMapTrackClass;
typedef struct _OsmGpsMapTrackPrivate OsmGpsMapTrackPrivate;

struct _OsmGpsMapTrack
{
    GObject parent;

    OsmGpsMapTrackPrivate *priv;
};

struct _OsmGpsMapTrackClass
{
    GObjectClass parent_class;
};

/**
 * osm_gps_map_track_get_type:
 *
 * Get track type
 *
 * Return value: (element-type GType): The type of the track
 * Since: 0.7.0
 **/
GType osm_gps_map_track_get_type (void) G_GNUC_CONST;

/**
 * osm_gps_map_track_new:
 *
 * Create new track
 *
 * Returns: (transfer full): New track
 * Since: 0.7.0
 **/
OsmGpsMapTrack *    osm_gps_map_track_new           (void);
/**
 * osm_gps_map_track_add_point:
 * @track: (in): a #OsmGpsMapTrack
 * @point: (in): a #OsmGpsMapPoint point to add
 *
 * Add a point to track
 *
 * Since: 0.7.0
 **/
void                osm_gps_map_track_add_point     (OsmGpsMapTrack *track, const OsmGpsMapPoint *point);
/**
 * osm_gps_map_track_get_points:
 * @track: (in): a #OsmGpsMapTrack
 *
 * Get list of points in the track
 *
 * Returns: (element-type OsmGpsMapPoint) (transfer none): list of #OsmGpsMapPoint
 * Since: 0.7.0
 **/
GSList *            osm_gps_map_track_get_points    (OsmGpsMapTrack *track);
/**
 * osm_gps_map_track_set_color:
 * @track: (in): a #OsmGpsMapTrack
 * @color: (in): new track color
 *
 * Set track color
 *
 * Since: 1.1.0
 **/
void                osm_gps_map_track_set_color     (OsmGpsMapTrack *track, GdkRGBA *color);
/**
 * osm_gps_map_track_get_color:
 * @track: (in): a #OsmGpsMapTrack
 * @color: (out caller-allocates): track color
 *
 * Get track color
 *
 * Since: 0.7.0
 **/
void                osm_gps_map_track_get_color     (OsmGpsMapTrack *track, GdkRGBA *color);
void                osm_gps_map_track_set_highlight_color (OsmGpsMapTrack *track, GdkRGBA *color);
void                osm_gps_map_track_get_highlight_color (OsmGpsMapTrack *track, GdkRGBA *color);

/**
 * osm_gps_map_track_remove_point:
 * @track: (in): a #OsmGpsMapTrack
 * @pos: Position of the point to remove
 *
 * Remove track point at @pos position in point list
 *
 * Since: 1.1.0
 **/
void                osm_gps_map_track_remove_point(OsmGpsMapTrack* track, int pos);

/**
 * osm_gps_map_track_n_points:
 * @track: a #OsmGpsMapTrack
 *
 * Get number of points in the track
 *
 * Returns: the number of points in the track.
 * Since: 1.1.0
 **/
int                 osm_gps_map_track_n_points(OsmGpsMapTrack* track);

/**
 * osm_gps_map_track_insert_point:
 * @track: a #OsmGpsMapTrack
 * @np: a #OsmGpsMapPoint
 * @pos: Position for the point
 *
 * Instert point @np at given postition @pos
 *
 * Since: 1.1.0
 **/
void                osm_gps_map_track_insert_point(OsmGpsMapTrack* track, OsmGpsMapPoint* np, int pos);

/**
 * osm_gps_map_track_get_point:
 * @track: a #OsmGpsMapTrack
 * @pos: Position of the point to get
 *
 * Get a #OsmGpsMapPoint point at @pos of given track
 *
 * Returns: (transfer none): a #OsmGpsMapPoint
 * Since: 1.1.0
 **/
OsmGpsMapPoint*     osm_gps_map_track_get_point(OsmGpsMapTrack* track, int pos);

/**
 * osm_gps_map_track_get_length:
 * @track: (in): a #OsmGpsMapTrack
 *
 * Get track length in meters
 *
 * Returns: the length of the track in meters.
 * Since: 1.1.0
 **/
double              osm_gps_map_track_get_length(OsmGpsMapTrack* track);

/**
 * osm_gps_map_track_set_highlight_point:
 * Mark the given point as highlighted on this track.
 * @note The point must match a point in the list of points associated with the given track.
 */
void                osm_gps_map_track_set_highlight_point(OsmGpsMapTrack* track, OsmGpsMapPoint *point);
OsmGpsMapPoint *    osm_gps_map_track_get_highlight_point(OsmGpsMapTrack* track);

G_END_DECLS

#endif /* _OSM_GPS_MAP_TRACK_H */
