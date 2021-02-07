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

/**
 * SECTION:osm-gps-map-point
 * @short_description: A geographic location (latitude, longitude)
 * @stability: Stable
 * @include: osm-gps-map.h
 *
 * #OsmGpsMapPoint describes a geographic location (latitude, longitude).
 * Helper functions exist to create such a point from either radian co-ordinates
 * (osm_gps_map_point_new_radians()) or degrees (osm_gps_map_new_degrees()).
 **/

#include "converter.h"
#include "osm-gps-map-point.h"

GType
osm_gps_map_point_get_type (void)
{
    static GType our_type = 0;

    if (our_type == 0)
        our_type = g_boxed_type_register_static (g_intern_static_string ("OsmGpsMapPoint"),
				         (GBoxedCopyFunc)osm_gps_map_point_copy,
				         (GBoxedFreeFunc)osm_gps_map_point_free);
    return our_type;
}

/**
 * osm_gps_map_point_new_degrees:
 * @lat: (in): latitude in degrees
 * @lon: (in): longtitude in degrees
 *
 * Create point with specified params
 *
 * Returns: (transfer full): new point object
 * Since: 0.7.0
 **/
OsmGpsMapPoint *
osm_gps_map_point_new_degrees(float lat, float lon)
{
    OsmGpsMapPoint *p = g_new0(OsmGpsMapPoint, 1);
    p->rlat = deg2rad(lat);
    p->rlon = deg2rad(lon);
    p->user_data = NULL;
    return p;
}

/**
 * osm_gps_map_point_new_radians:
 * @rlat: (in): latitude in radians
 * @rlon: (in): longtitude in radians
 *
 * Create point with specified params
 *
 * Returns: (transfer full): new point object
 * Since: 0.7.0
 **/
OsmGpsMapPoint *
osm_gps_map_point_new_radians(float rlat, float rlon)
{
    OsmGpsMapPoint *p = g_new0(OsmGpsMapPoint, 1);
    p->rlat = rlat;
    p->rlon = rlon;
    p->user_data = NULL;
    return p;
}

/**
 * osm_gps_map_point_new_degrees_with_user_data:
 * @lat: (in): latitude in degrees
 * @lon: (in): longtitude in degrees
 * @user_data: (in): user data
 *
 * Create point with specified params
 *
 * Returns: (transfer full): new point object
 * Since: 1.2.0
 **/
OsmGpsMapPoint *
osm_gps_map_point_new_degrees_with_user_data(float lat, float lon, gpointer user_data)
{
    OsmGpsMapPoint *p = g_new0(OsmGpsMapPoint, 1);
    p->rlat = deg2rad(lat);
    p->rlon = deg2rad(lon);
    p->user_data = user_data;
    return p;
}

/**
 * osm_gps_map_point_new_radians_with_user_data:
 * @rlat: (in): latitude in radians
 * @rlon: (in): longtitude in radians
 * @user_data: (in): user data
 *
 * Create point with specified params
 *
 * Returns: (transfer full): new point object
 * Since: 1.2.0
 **/
OsmGpsMapPoint *
osm_gps_map_point_new_radians_with_user_data(float rlat, float rlon, gpointer user_data)
{
    OsmGpsMapPoint *p = g_new0(OsmGpsMapPoint, 1);
    p->rlat = rlat;
    p->rlon = rlon;
    p->user_data = user_data;
    return p;
}

/**
 * osm_gps_map_point_get_degrees:
 * @point: The point ( latitude and longitude in radian )
 * @lat: (out): latitude in degrees
 * @lon: (out): longitude in degrees
 *
 * Returns the lagitude and longitude in degrees.
 *
 * Since: 0.7.0
 **/
void
osm_gps_map_point_get_degrees(OsmGpsMapPoint *point, float *lat, float *lon)
{
    *lat = rad2deg(point->rlat);
    *lon = rad2deg(point->rlon);
}

/**
 * osm_gps_map_point_get_radians:
 * @point: The #OsmGpsMapPoint point ( latitude and longitude in radian )
 * @rlat: (out): latitude in radians
 * @rlon: (out): longitude in radians
 *
 * Returns the lagitude and longitude in radians.
 *
 * Since: 0.7.0
 **/
void
osm_gps_map_point_get_radians(OsmGpsMapPoint *point, float *rlat, float *rlon)
{
    *rlat = point->rlat;
    *rlon = point->rlon;
}

/**
 * osm_gps_map_point_set_degrees:
 * @point: The #OsmGpsMapPoint point ( latitude and longitude in radian )
 * @lat: (in): latitude in degrees
 * @lon: (in): longitude in degrees
 *
 * Sets the lagitude and longitude in degrees.
 *
 * Since: 0.7.0
 **/
void
osm_gps_map_point_set_degrees(OsmGpsMapPoint *point, float lat, float lon)
{
    point->rlat = deg2rad(lat);
    point->rlon = deg2rad(lon);
}

/**
 * osm_gps_map_point_set_radians:
 * @point: The #OsmGpsMapPoint point ( latitude and longitude in radian )
 * @rlat: (in): latitude in radians
 * @rlon: (in): longitude in radians
 *
 * Sets the lagitude and longitude in radians.
 *
 * Since: 0.7.0
 **/
void
osm_gps_map_point_set_radians(OsmGpsMapPoint *point, float rlat, float rlon)
{
    point->rlat = rlat;
    point->rlon = rlon;
}

/**
 * osm_gps_map_point_get_user_data:
 * @point: The #OsmGpsMapPoint point
 *
 * Get user data stored in point
 *
 * Returns: (transfer none): The #OsmGpsMapPoint user data
 * Since: 1.2.0
 **/
gpointer
osm_gps_map_point_get_user_data(OsmGpsMapPoint *point)
{
    return point->user_data;
}

/**
 * osm_gps_map_point_set_user_data:
 * @point: The #OsmGpsMapPoint point
 * @user_data: (in): user data
 *
 * Store user data in point
 *
 * Since: 1.2.0
 **/
void
osm_gps_map_point_set_user_data(OsmGpsMapPoint *point, gpointer user_data)
{
    point->user_data = user_data;
}

/**
 * osm_gps_map_point_copy:
 * @point: (in): a #OsmGpsMapPoint object
 *
 * Create a copy of a point
 *
 * Returns: (transfer full): Copied point
 * Since: 0.7.2
 */
OsmGpsMapPoint *
osm_gps_map_point_copy (const OsmGpsMapPoint *point)
{
    OsmGpsMapPoint *result = g_new (OsmGpsMapPoint, 1);
    *result = *point;

    return result;
}

/**
 * osm_gps_map_point_free:
 * @point: a #OsmGpsMapPoint object
 *
 * Free point object
 *
 * Since: 0.7.2
 */
void
osm_gps_map_point_free (OsmGpsMapPoint *point)
{
    g_free(point);
}
