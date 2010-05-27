#include "converter.h"
#include "osm-gps-map-compat.h"

/**
 * osm_gps_map_draw_gps:
 *
 * Deprecated: Use osm_gps_map_gps_add() instead.
 **/
void
osm_gps_map_draw_gps (OsmGpsMap *map, float latitude, float longitude, float heading)
{
    g_critical("%s is deprecated", G_STRFUNC);
    osm_gps_map_gps_add (map, latitude, longitude, heading);
}

/**
 * osm_gps_map_clear_gps:
 *
 * Deprecated: Use osm_gps_map_gps_clear() instead.
 **/
void
osm_gps_map_clear_gps (OsmGpsMap *map)
{
    g_critical("%s is deprecated", G_STRFUNC);
    osm_gps_map_gps_clear (map);
}

/**
 * osm_gps_map_add_track:
 *
 * Deprecated: Use osm_gps_map_track_add() instead.
 **/
void
osm_gps_map_add_track (OsmGpsMap *map, GSList *list)
{
    OsmGpsMapTrack *track;
    g_critical("%s is deprecated", G_STRFUNC);
    track = g_object_new (OSM_TYPE_GPS_MAP_TRACK, "track", list, NULL);
    osm_gps_map_track_add (map, track);
}

/**
 * osm_gps_map_replace_track:
 *
 * Deprecated: Use osm_gps_map_track_remove() and osm_gps_map_track_add() or just
 * edit the #OsmGpsMapTrack object directly
 **/
void
osm_gps_map_replace_track (OsmGpsMap *map, GSList *old_track, GSList *new_track)
{
    g_critical("%s is deprecated", G_STRFUNC);
}

/**
 * osm_gps_map_clear_tracks:
 *
 * Deprecated: Use osm_gps_map_track_remove_all() instead.
 **/
void
osm_gps_map_clear_tracks (OsmGpsMap *map)
{
    g_critical("%s is deprecated", G_STRFUNC);
    osm_gps_map_track_remove_all (map);
}

/**
 * osm_gps_map_add_image_with_alignment:
 *
 * Deprecated: Use osm_gps_map_image_add_with_alignment() instead.
 **/
void
osm_gps_map_add_image_with_alignment (OsmGpsMap *map, float latitude, float longitude, GdkPixbuf *image, float xalign, float yalign)
{
    g_critical("%s is deprecated", G_STRFUNC);
    osm_gps_map_image_add_with_alignment (map, latitude, longitude, image, xalign, yalign);
}

/**
 * osm_gps_map_add_image:
 *
 * Deprecated: Use osm_gps_map_image_add() instead.
 **/
void
osm_gps_map_add_image (OsmGpsMap *map, float latitude, float longitude, GdkPixbuf *image)
{
    g_critical("%s is deprecated", G_STRFUNC);
    osm_gps_map_image_add_with_alignment (map, latitude, longitude, image, 0.5, 0.5);
}

/**
 * osm_gps_map_clear_images:
 *
 * Deprecated: Use osm_gps_map_image_remove_all() instead.
 **/
void
osm_gps_map_clear_images (OsmGpsMap *map)
{
    g_critical("%s is deprecated", G_STRFUNC);
    osm_gps_map_image_remove_all (map);
}

/**
 * osm_gps_map_remove_image:
 *
 * Deprecated: Use osm_gps_map_image_remove() instead.
 **/
gboolean
osm_gps_map_remove_image (OsmGpsMap *map, GdkPixbuf *image)
{
    g_critical("%s is deprecated", G_STRFUNC);
    return FALSE;
}

/**
 * osm_gps_map_add_layer:
 *
 * Deprecated: Use osm_gps_map_layer_add() instead.
 **/
void
osm_gps_map_add_layer (OsmGpsMap *map, OsmGpsMapLayer *layer)
{
    g_critical("%s is deprecated", G_STRFUNC);
    osm_gps_map_layer_add (map, layer);
}

/**
 * osm_gps_map_screen_to_geographic:
 *
 * Deprecated: Use osm_gps_map_convert_screen_to_geographic() instead.
 **/
void
osm_gps_map_screen_to_geographic (OsmGpsMap *map, gint pixel_x, gint pixel_y,
                                  gfloat *latitude, gfloat *longitude)
{
    OsmGpsMapPoint p;
    g_critical("%s is deprecated", G_STRFUNC);
    osm_gps_map_convert_screen_to_geographic(map, pixel_x, pixel_y, &p);
    if (latitude)
        *latitude = rad2deg(p.rlat);
    if (longitude)
        *longitude = rad2deg(p.rlon);
}

/**
 * osm_gps_map_geographic_to_screen:
 *
 * Deprecated: Use osm_gps_map_convert_geographic_to_screen() instead.
 **/
void
osm_gps_map_geographic_to_screen (OsmGpsMap *map,
                                  gfloat latitude, gfloat longitude,
                                  gint *pixel_x, gint *pixel_y)
{
    OsmGpsMapPoint p;
    g_critical("%s is deprecated", G_STRFUNC);
    osm_gps_map_point_set_degrees(&p, latitude, longitude);
    osm_gps_map_convert_geographic_to_screen(map, &p, pixel_x, pixel_y);
}

/**
 * osm_gps_map_get_co_ordinates:
 *
 * Deprecated: Use osm_gps_map_convert_screen_to_geographic() or 
 * osm_gps_map_get_event_location() instead.
 **/
OsmGpsMapPoint
osm_gps_map_get_co_ordinates (OsmGpsMap *map, int pixel_x, int pixel_y)
{
    OsmGpsMapPoint p;
    g_critical("%s is deprecated", G_STRFUNC);
    osm_gps_map_convert_screen_to_geographic(map, pixel_x, pixel_y, &p);
    return p;
}
