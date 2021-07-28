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

#ifndef _OSM_GPS_MAP_SOURCE_H_
#define _OSM_GPS_MAP_SOURCE_H_

#include <glib.h>

G_BEGIN_DECLS

typedef enum {
    OSM_GPS_MAP_SOURCE_NULL,
    OSM_GPS_MAP_SOURCE_OPENSTREETMAP,
    OSM_GPS_MAP_SOURCE_OPENSTREETMAP_RENDERER,
    OSM_GPS_MAP_SOURCE_OPENAERIALMAP,
    OSM_GPS_MAP_SOURCE_MAPS_FOR_FREE,
    OSM_GPS_MAP_SOURCE_OPENCYCLEMAP,
    OSM_GPS_MAP_SOURCE_OPENTOPOMAP,
    OSM_GPS_MAP_SOURCE_OSM_PUBLIC_TRANSPORT,
    OSM_GPS_MAP_SOURCE_GOOGLE_STREET,
    OSM_GPS_MAP_SOURCE_GOOGLE_SATELLITE,
    OSM_GPS_MAP_SOURCE_GOOGLE_HYBRID,
    OSM_GPS_MAP_SOURCE_VIRTUAL_EARTH_STREET,
    OSM_GPS_MAP_SOURCE_VIRTUAL_EARTH_SATELLITE,
    OSM_GPS_MAP_SOURCE_VIRTUAL_EARTH_HYBRID,
    OSM_GPS_MAP_SOURCE_OSMC_TRAILS,

    OSM_GPS_MAP_SOURCE_LAST
} OsmGpsMapSource_t;

/**
 * osm_gps_map_source_get_friendly_name:
 * @source: (in): a #OsmGpsMapSource_t source id
 *
 * Get friendly name for source
 *
 * Returns: Nice printable source name
 * Since: 0.7.0
 **/
const char* osm_gps_map_source_get_friendly_name    (OsmGpsMapSource_t source);
/**
 * osm_gps_map_source_get_copyright:
 * @source: (in): a #OsmGpsMapSource_t source id
 *
 * Get copyright information for the source
 *
 * Returns: Source copyright information
 * Since: 1.2.0
 **/
const char* osm_gps_map_source_get_copyright        (OsmGpsMapSource_t source);
/**
 * osm_gps_map_source_get_repo_uri:
 * @source: (in): a #OsmGpsMapSource_t source id
 *
 * Get repository URI address for the source
 *
 * Returns: Source repo URI
 * Since: 0.7.0
 **/
const char* osm_gps_map_source_get_repo_uri         (OsmGpsMapSource_t source);
/**
 * osm_gps_map_source_get_image_format:
 * @source: (in): a #OsmGpsMapSource_t source id
 *
 * Get tile image format for the source
 *
 * Returns: source tile image format
 * Since: 0.7.0
 **/
const char* osm_gps_map_source_get_image_format     (OsmGpsMapSource_t source);
/**
 * osm_gps_map_source_get_min_zoom:
 * @source: (in): a #OsmGpsMapSource_t source id
 *
 * Get minimum zoom level for the source
 *
 * Returns: source minimum zoom level
 * Since: 0.7.0
 **/
int         osm_gps_map_source_get_min_zoom         (OsmGpsMapSource_t source);
/**
 * osm_gps_map_source_get_max_zoom:
 * @source: (in): a #OsmGpsMapSource_t source id
 *
 * Get maximum zoom level for the source
 *
 * Returns: source maximum zoom level
 * Since: 0.7.0
 **/
int         osm_gps_map_source_get_max_zoom         (OsmGpsMapSource_t source);
/**
 * osm_gps_map_source_is_valid:
 * @source: (in): a #OsmGpsMapSource_t source id
 *
 * Check whether source is considered valid
 *
 * Returns: Validity of the source (whether repo uri is not null)
 * Since: 0.7.0
 **/
gboolean    osm_gps_map_source_is_valid             (OsmGpsMapSource_t source);

G_END_DECLS

#endif /* _OSM_GPS_MAP_SOURCE_H_ */
