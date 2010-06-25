/* osm-gps-map-point.c */

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

OsmGpsMapPoint *
osm_gps_map_point_new_degrees(float lat, float lon)
{
    OsmGpsMapPoint *p = g_new0(OsmGpsMapPoint, 1);
    p->rlat = deg2rad(lat);
    p->rlon = deg2rad(lon);
    return p;
}

OsmGpsMapPoint *
osm_gps_map_point_new_radians(float rlat, float rlon)
{
    OsmGpsMapPoint *p = g_new0(OsmGpsMapPoint, 1);
    p->rlat = rlat;
    p->rlon = rlon;
    return p;
}

void
osm_gps_map_point_get_degrees(OsmGpsMapPoint *point, float *lat, float *lon)
{
    *lat = rad2deg(point->rlat);
    *lon = rad2deg(point->rlon);
}

void
osm_gps_map_point_get_radians(OsmGpsMapPoint *point, float *rlat, float *rlon)
{
    *rlat = point->rlat;
    *rlon = point->rlon;
}

void
osm_gps_map_point_set_degrees(OsmGpsMapPoint *point, float lat, float lon)
{
    point->rlat = deg2rad(lat);
    point->rlon = deg2rad(lon);
}

void
osm_gps_map_point_set_radians(OsmGpsMapPoint *point, float rlat, float rlon)
{
    point->rlat = rlat;
    point->rlon = rlon;
}

/**
 * osm_gps_map_point_copy:
 *
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
 *
 * Since: 0.7.2
 */
void
osm_gps_map_point_free (OsmGpsMapPoint *point)
{
    g_free(point);
}
