/* osm-gps-map-point.c */

#include "converter.h"
#include "osm-gps-map-point.h"

OsmGpsMapPoint *
osm_gps_map_point_new_degrees(gdouble lat, gdouble lon)
{
    OsmGpsMapPoint *p = g_new0(OsmGpsMapPoint, 1);
    p->rlat = deg2rad(lat);
    p->rlon = deg2rad(lon);
    return p;
}

OsmGpsMapPoint *
osm_gps_map_point_new_radians(gdouble rlat, gdouble rlon)
{
    OsmGpsMapPoint *p = g_new0(OsmGpsMapPoint, 1);
    p->rlat = rlat;
    p->rlon = rlon;
    return p;
}

void
osm_gps_map_point_as_degrees(OsmGpsMapPoint *point, gdouble *lat, gdouble *lon)
{
    *lat = rad2deg(point->rlat);
    *lon = rad2deg(point->rlon);
}

void
osm_gps_map_point_as_radians(OsmGpsMapPoint *point, gdouble *rlat, gdouble *rlon)
{
    *rlat = point->rlat;
    *rlon = point->rlon;
}

static OsmGpsMapPoint *
osm_gps_map_point_copy (const OsmGpsMapPoint *point)
{
    OsmGpsMapPoint *result = g_new (OsmGpsMapPoint, 1);
    *result = *point;

    return result;
}

GType
osm_gps_map_point_get_type (void)
{
    static GType our_type = 0;

    if (our_type == 0)
        our_type = g_boxed_type_register_static (g_intern_static_string ("OsmGpsMapPoint"),
				         (GBoxedCopyFunc)osm_gps_map_point_copy,
				         (GBoxedFreeFunc)g_free);
    return our_type;
}
