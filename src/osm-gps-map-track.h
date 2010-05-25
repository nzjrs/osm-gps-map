/* osm-gps-map-track.h */

#ifndef _OSM_GPS_MAP_TRACK_H
#define _OSM_GPS_MAP_TRACK_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define OSM_TYPE_GPS_MAP_TRACK              osm_gps_map_track_get_type()
#define OSM_GPS_MAP_TRACK(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), OSM_TYPE_GPS_MAP_TRACK, OsmGpsMapTrack))
#define OSM_GPS_MAP_TRACK_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), OSM_TYPE_GPS_MAP_TRACK, OsmGpsMapTrackClass))
#define OSM_IS_GPS_MAP_TRACK(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), OSM_TYPE_GPS_MAP_TRACK))
#define OSM_IS_GPS_MAP_TRACK_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), OSM_TYPE_GPS_MAP_TRACK))
#define OSM_GPS_MAP_TRACK_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), OSM_TYPE_GPS_MAP_TRACK, OsmGpsMapTrackClass))

#define OSM_TYPE_GPS_MAP_POINT              osm_gps_map_point_get_type()

typedef struct _OsmGpsMapTrack OsmGpsMapTrack;
typedef struct _OsmGpsMapTrackClass OsmGpsMapTrackClass;
typedef struct _OsmGpsMapTrackPrivate OsmGpsMapTrackPrivate;
typedef struct _OsmGpsMapPoint OsmGpsMapPoint;

struct _OsmGpsMapTrack
{
    GObject parent;

    OsmGpsMapTrackPrivate *priv;
};

struct _OsmGpsMapTrackClass
{
    GObjectClass parent_class;
};

struct _OsmGpsMapPoint
{
    /* radians */
    gdouble  rlat;
    gdouble  rlon;
};

GType osm_gps_map_track_get_type (void) G_GNUC_CONST;
GType osm_gps_map_point_get_type (void) G_GNUC_CONST;

OsmGpsMapTrack *    osm_gps_map_track_new           (void);
void                osm_gps_map_track_add_point     (OsmGpsMapTrack *track, OsmGpsMapPoint *point);
GSList *            osm_gps_map_track_get_points    (OsmGpsMapTrack *track);

OsmGpsMapPoint *    osm_gps_map_point_new_degrees   (gdouble lat, gdouble lon);
OsmGpsMapPoint *    osm_gps_map_point_new_radians   (gdouble rlat, gdouble rlon);
void                osm_gps_map_point_as_degrees    (OsmGpsMapPoint *point, gdouble *lat, gdouble *lon);
void                osm_gps_map_point_as_radians    (OsmGpsMapPoint *point, gdouble *rlat, gdouble *rlon);

G_END_DECLS

#endif /* _OSM_GPS_MAP_TRACK_H */
