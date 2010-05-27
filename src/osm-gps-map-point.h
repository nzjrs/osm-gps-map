/* osm-gps-map-point.h */

#ifndef _OSM_GPS_MAP_POINT_H
#define _OSM_GPS_MAP_POINT_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define OSM_TYPE_GPS_MAP_POINT              osm_gps_map_point_get_type()

typedef struct _OsmGpsMapPoint OsmGpsMapPoint;

struct _OsmGpsMapPoint
{
    /* radians */
    gdouble  rlat;
    gdouble  rlon;
};

GType osm_gps_map_point_get_type (void) G_GNUC_CONST;

OsmGpsMapPoint *    osm_gps_map_point_new_degrees   (gdouble lat, gdouble lon);
OsmGpsMapPoint *    osm_gps_map_point_new_radians   (gdouble rlat, gdouble rlon);
void                osm_gps_map_point_as_degrees    (OsmGpsMapPoint *point, gdouble *lat, gdouble *lon);
void                osm_gps_map_point_as_radians    (OsmGpsMapPoint *point, gdouble *rlat, gdouble *rlon);

G_END_DECLS

#endif /* _OSM_GPS_MAP_POINT_H */
