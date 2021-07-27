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

#include "private.h"
#include "osm-gps-map-source.h"

const char*
osm_gps_map_source_get_friendly_name(OsmGpsMapSource_t source)
{
    switch(source)
    {
        case OSM_GPS_MAP_SOURCE_NULL:
            return "None";
        case OSM_GPS_MAP_SOURCE_OPENSTREETMAP:
            return "OpenStreetMap I";
        case OSM_GPS_MAP_SOURCE_OPENSTREETMAP_RENDERER:
            return "OpenStreetMap II";
        case OSM_GPS_MAP_SOURCE_OPENAERIALMAP:
            return "OpenAerialMap";
        case OSM_GPS_MAP_SOURCE_OPENCYCLEMAP:
            return "OpenCycleMap";
        case OSM_GPS_MAP_SOURCE_OPENTOPOMAP: 
            return "OpenTopoMap";
        case OSM_GPS_MAP_SOURCE_OSM_PUBLIC_TRANSPORT:
            return "Public Transport";
        case OSM_GPS_MAP_SOURCE_OSMC_TRAILS:
            return "OSMC Trails";
        case OSM_GPS_MAP_SOURCE_MAPS_FOR_FREE:
            return "Maps-For-Free";
        case OSM_GPS_MAP_SOURCE_GOOGLE_STREET:
            return "Google Maps";
        case OSM_GPS_MAP_SOURCE_GOOGLE_SATELLITE:
            return "Google Satellite";
        case OSM_GPS_MAP_SOURCE_GOOGLE_HYBRID:
            return "Google Hybrid";
        case OSM_GPS_MAP_SOURCE_VIRTUAL_EARTH_STREET:
            return "Virtual Earth";
        case OSM_GPS_MAP_SOURCE_VIRTUAL_EARTH_SATELLITE:
            return "Virtual Earth Satellite";
        case OSM_GPS_MAP_SOURCE_VIRTUAL_EARTH_HYBRID:
            return "Virtual Earth Hybrid";
        case OSM_GPS_MAP_SOURCE_LAST:
        default:
            return NULL;
    }
    return NULL;
}

const char*
osm_gps_map_source_get_copyright(OsmGpsMapSource_t source)
{
    switch(source)
    {
        case OSM_GPS_MAP_SOURCE_OPENSTREETMAP:
            // https://www.openstreetmap.org/copyright
            return "© OpenStreetMap contributors";
        case OSM_GPS_MAP_SOURCE_OPENCYCLEMAP:
            // http://www.thunderforest.com/terms/
            return "Maps © thunderforest.com, Data © osm.org/copyright";
        case OSM_GPS_MAP_SOURCE_OSM_PUBLIC_TRANSPORT:
            return "Maps © ÖPNVKarte, Data © OpenStreetMap contributors";
        case OSM_GPS_MAP_SOURCE_MAPS_FOR_FREE:
            return "Maps © Maps-For-Free";
        case OSM_GPS_MAP_SOURCE_OPENTOPOMAP:
            return "© OpenTopoMap (CC-BY-SA)";
        case OSM_GPS_MAP_SOURCE_GOOGLE_STREET:
            return "Map provided by Google";
        case OSM_GPS_MAP_SOURCE_GOOGLE_SATELLITE:
            return "Map provided by Google ";
        case OSM_GPS_MAP_SOURCE_GOOGLE_HYBRID:
            return "Map provided by Google";
        case OSM_GPS_MAP_SOURCE_VIRTUAL_EARTH_STREET:
            return "Map provided by Microsoft";
        case OSM_GPS_MAP_SOURCE_VIRTUAL_EARTH_SATELLITE:
            return "Map provided by Microsoft";
        case OSM_GPS_MAP_SOURCE_VIRTUAL_EARTH_HYBRID:
            return "Map provided by Microsoft";
        default:
            return NULL;
    }
    return NULL;
}

//http://www.internettablettalk.com/forums/showthread.php?t=5209
//https://garage.maemo.org/plugins/scmsvn/viewcvs.php/trunk/src/maps.c?root=maemo-mapper&view=markup
//http://www.ponies.me.uk/maps/GoogleTileUtils.java
//http://www.mgmaps.com/cache/MapTileCacher.perl
const char*
osm_gps_map_source_get_repo_uri(OsmGpsMapSource_t source)
{
    switch(source)
    {
        case OSM_GPS_MAP_SOURCE_NULL:
            return "none://";
        case OSM_GPS_MAP_SOURCE_OPENSTREETMAP:
            return OSM_REPO_URI;
        case OSM_GPS_MAP_SOURCE_OPENAERIALMAP:
            /* OpenAerialMap is down, offline till furthur notice
               http://openaerialmap.org/pipermail/talk_openaerialmap.org/2008-December/000055.html */
            return NULL;
        case OSM_GPS_MAP_SOURCE_OPENSTREETMAP_RENDERER:
            /* The Tile@Home serverhas been shut down. */
            // return "http://tah.openstreetmap.org/Tiles/tile/#Z/#X/#Y.png";
            return NULL;
        case OSM_GPS_MAP_SOURCE_OPENCYCLEMAP:
            // return "http://c.andy.sandbox.cloudmade.com/tiles/cycle/#Z/#X/#Y.png";
            return "http://b.tile.opencyclemap.org/cycle/#Z/#X/#Y.png";
        case OSM_GPS_MAP_SOURCE_OSM_PUBLIC_TRANSPORT:
            return "http://tile.xn--pnvkarte-m4a.de/tilegen/#Z/#X/#Y.png";
        case OSM_GPS_MAP_SOURCE_OSMC_TRAILS:
            // Appears to be shut down
            return NULL;
        case OSM_GPS_MAP_SOURCE_MAPS_FOR_FREE:
            return "https://maps-for-free.com/layer/relief/z#Z/row#Y/#Z_#X-#Y.jpg";
        case OSM_GPS_MAP_SOURCE_OPENTOPOMAP:
            return "https://a.tile.opentopomap.org/#Z/#X/#Y.png";
        case OSM_GPS_MAP_SOURCE_GOOGLE_STREET:
            return "http://mt#R.google.com/vt/lyrs=m&hl=en&x=#X&s=&y=#Y&z=#Z";
        case OSM_GPS_MAP_SOURCE_GOOGLE_HYBRID:
            return "http://mt#R.google.com/vt/lyrs=y&hl=en&x=#X&s=&y=#Y&z=#Z";
        case OSM_GPS_MAP_SOURCE_GOOGLE_SATELLITE:
            return "http://mt#R.google.com/vt/lyrs=s&hl=en&x=#X&s=&y=#Y&z=#Z";
        case OSM_GPS_MAP_SOURCE_VIRTUAL_EARTH_STREET:
            return "http://a#R.ortho.tiles.virtualearth.net/tiles/r#W.jpeg?g=50";
        case OSM_GPS_MAP_SOURCE_VIRTUAL_EARTH_SATELLITE:
            return "http://a#R.ortho.tiles.virtualearth.net/tiles/a#W.jpeg?g=50";
        case OSM_GPS_MAP_SOURCE_VIRTUAL_EARTH_HYBRID:
            return "http://a#R.ortho.tiles.virtualearth.net/tiles/h#W.jpeg?g=50";
        case OSM_GPS_MAP_SOURCE_LAST:
        default:
            return NULL;
    }
    return NULL;
}

const char *
osm_gps_map_source_get_image_format(OsmGpsMapSource_t source)
{
    switch(source) {
        case OSM_GPS_MAP_SOURCE_NULL:
        case OSM_GPS_MAP_SOURCE_OPENSTREETMAP:
        case OSM_GPS_MAP_SOURCE_OPENSTREETMAP_RENDERER:
        case OSM_GPS_MAP_SOURCE_OPENCYCLEMAP:
        case OSM_GPS_MAP_SOURCE_OSM_PUBLIC_TRANSPORT:
        case OSM_GPS_MAP_SOURCE_OSMC_TRAILS:
        case OSM_GPS_MAP_SOURCE_OPENTOPOMAP:
            return "png";
        case OSM_GPS_MAP_SOURCE_OPENAERIALMAP:
        case OSM_GPS_MAP_SOURCE_GOOGLE_STREET:
        case OSM_GPS_MAP_SOURCE_GOOGLE_HYBRID:
        case OSM_GPS_MAP_SOURCE_VIRTUAL_EARTH_STREET:
        case OSM_GPS_MAP_SOURCE_VIRTUAL_EARTH_SATELLITE:
        case OSM_GPS_MAP_SOURCE_VIRTUAL_EARTH_HYBRID:
        case OSM_GPS_MAP_SOURCE_MAPS_FOR_FREE:
        case OSM_GPS_MAP_SOURCE_GOOGLE_SATELLITE:
            return "jpg";
        case OSM_GPS_MAP_SOURCE_LAST:
        default:
            return "bin";
    }
    return "bin";
}


int
osm_gps_map_source_get_min_zoom(OsmGpsMapSource_t source)
{
    return 1;
}

int
osm_gps_map_source_get_max_zoom(OsmGpsMapSource_t source)
{
    switch(source) {
        case OSM_GPS_MAP_SOURCE_NULL:
            return 18;
        case OSM_GPS_MAP_SOURCE_OPENSTREETMAP:
            return 19;
        case OSM_GPS_MAP_SOURCE_OPENCYCLEMAP:
            return 18;
        case OSM_GPS_MAP_SOURCE_OSM_PUBLIC_TRANSPORT:
            return OSM_MAX_ZOOM;
        case OSM_GPS_MAP_SOURCE_OPENSTREETMAP_RENDERER:
        case OSM_GPS_MAP_SOURCE_OPENAERIALMAP:
        case OSM_GPS_MAP_SOURCE_OPENTOPOMAP:
            return 17;
        case OSM_GPS_MAP_SOURCE_GOOGLE_STREET:
        case OSM_GPS_MAP_SOURCE_GOOGLE_SATELLITE:
        case OSM_GPS_MAP_SOURCE_GOOGLE_HYBRID:
            return OSM_MAX_ZOOM;
        case OSM_GPS_MAP_SOURCE_VIRTUAL_EARTH_STREET:
        case OSM_GPS_MAP_SOURCE_VIRTUAL_EARTH_SATELLITE:
        case OSM_GPS_MAP_SOURCE_VIRTUAL_EARTH_HYBRID:
            return 19;
        case OSM_GPS_MAP_SOURCE_OSMC_TRAILS:
            return 15;
        case OSM_GPS_MAP_SOURCE_MAPS_FOR_FREE:
            return 11;
        case OSM_GPS_MAP_SOURCE_LAST:
        default:
            return 17;
    }
    return 17;
}

gboolean
osm_gps_map_source_is_valid(OsmGpsMapSource_t source)
{
    return osm_gps_map_source_get_repo_uri(source) != NULL;
}
