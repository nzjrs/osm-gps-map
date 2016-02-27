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
 * SECTION:osm-gps-map-track
 * @short_description: A list of GPS points
 * @stability: Stable
 * @include: osm-gps-map.h
 *
 * #OsmGpsMapTrack stores multiple #OsmGpsMapPoint objects, i.e. a track, and
 * describes how such a track should be drawn on the map
 * (see osm_gps_map_track_add()), including its colour, width, etc.
 **/

#include <gdk/gdk.h>
#include <math.h>

#include "converter.h"
#include "osm-gps-map-track.h"

G_DEFINE_TYPE (OsmGpsMapTrack, osm_gps_map_track, G_TYPE_OBJECT)

enum
{
    PROP_0,
    PROP_VISIBLE,
    PROP_TRACK,
    PROP_LINE_WIDTH,
    PROP_ALPHA,
    PROP_COLOR,
    PROP_EDITABLE
};

enum
{
    POINT_ADDED,
    POINT_CHANGED,
    POINT_INSERTED,
    POINT_REMOVED,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = {0,};

struct _OsmGpsMapTrackPrivate
{
    GSList *track;
    gboolean visible;
    gfloat linewidth;
    gfloat alpha;
    GdkRGBA color;
    gboolean editable;
};

#define DEFAULT_R   (0.6)
#define DEFAULT_G   (0)
#define DEFAULT_B   (0)
#define DEFAULT_A   (0.6)

static void
osm_gps_map_track_get_property (GObject    *object,
                                guint       property_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
    OsmGpsMapTrackPrivate *priv = OSM_GPS_MAP_TRACK(object)->priv;

    switch (property_id)
    {
        case PROP_VISIBLE:
            g_value_set_boolean(value, priv->visible);
            break;
        case PROP_TRACK:
            g_value_set_pointer(value, priv->track);
            break;
        case PROP_LINE_WIDTH:
            g_value_set_float(value, priv->linewidth);
            break;
        case PROP_ALPHA:
            g_value_set_float(value, priv->alpha);
            break;
        case PROP_COLOR:
            g_value_set_boxed(value, &priv->color);
            break;
        case PROP_EDITABLE:
            g_value_set_boolean(value, priv->editable);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
osm_gps_map_track_set_property (GObject      *object,
                                guint         property_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
    OsmGpsMapTrackPrivate *priv = OSM_GPS_MAP_TRACK(object)->priv;

    switch (property_id)
    {
        case PROP_VISIBLE:
            priv->visible = g_value_get_boolean (value);
            break;
        case PROP_TRACK:
            priv->track = g_value_get_pointer (value);
            break;
        case PROP_LINE_WIDTH:
            priv->linewidth = g_value_get_float (value);
            break;
        case PROP_ALPHA:
            priv->alpha = g_value_get_float (value);
            break;
        case PROP_COLOR: {
            GdkRGBA *c = g_value_get_boxed (value);
            priv->color.red = c->red;
            priv->color.green = c->green;
            priv->color.blue = c->blue;
            } break;
        case PROP_EDITABLE:
            priv->editable = g_value_get_boolean(value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
osm_gps_map_track_dispose (GObject *object)
{
    g_return_if_fail (OSM_IS_GPS_MAP_TRACK (object));
    OsmGpsMapTrackPrivate *priv = OSM_GPS_MAP_TRACK(object)->priv;

    if (priv->track) {
        g_slist_foreach(priv->track, (GFunc) g_free, NULL);
        g_slist_free(priv->track);
        priv->track = NULL;
    }

    G_OBJECT_CLASS (osm_gps_map_track_parent_class)->dispose (object);
}

static void
osm_gps_map_track_finalize (GObject *object)
{
    G_OBJECT_CLASS (osm_gps_map_track_parent_class)->finalize (object);
}

static void
osm_gps_map_track_class_init (OsmGpsMapTrackClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (klass, sizeof (OsmGpsMapTrackPrivate));

    object_class->get_property = osm_gps_map_track_get_property;
    object_class->set_property = osm_gps_map_track_set_property;
    object_class->dispose = osm_gps_map_track_dispose;
    object_class->finalize = osm_gps_map_track_finalize;

    g_object_class_install_property (object_class,
                                     PROP_VISIBLE,
                                     g_param_spec_boolean ("visible",
                                                           "visible",
                                                           "should this track be visible",
                                                           TRUE,
                                                           G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (object_class,
                                     PROP_TRACK,
                                     g_param_spec_pointer ("track",
                                                           "track",
                                                           "list of points for the track",
                                                           G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

    g_object_class_install_property (object_class,
                                     PROP_LINE_WIDTH,
                                     g_param_spec_float ("line-width",
                                                         "line-width",
                                                         "width of the lines drawn for the track",
                                                         0.0,       /* minimum property value */
                                                         100.0,     /* maximum property value */
                                                         4.0,
                                                         G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (object_class,
                                     PROP_ALPHA,
                                     g_param_spec_float ("alpha",
                                                         "alpha",
                                                         "alpha transparency of the track",
                                                         0.0,       /* minimum property value */
                                                         1.0,       /* maximum property value */
                                                         DEFAULT_A,
                                                         G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (object_class,
                                     PROP_COLOR,
                                     g_param_spec_boxed ("color",
                                                         "color",
                                                         "color of the track",
                                                         GDK_TYPE_RGBA,
                                                         G_PARAM_READABLE | G_PARAM_WRITABLE));

    g_object_class_install_property (object_class,
                                     PROP_EDITABLE,
                                     g_param_spec_boolean ("editable",
                                                           "editable",
                                                           "should this track be editable",
                                                           FALSE,
                                                           G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));

    /**
    * OsmGpsMapTrack::point-added:
    * @self: A #OsmGpsMapTrack
    * @arg1: The added #OsmGpsMapPoint
    *
    * The #OsmGpsMapTrack::point-added signal is emitted whenever a #OsmGpsMapPoint
    * is added to the #OsmGpsMapTrack.
    */
	signals [POINT_ADDED] = g_signal_new ("point-added",
	                            OSM_TYPE_GPS_MAP_TRACK,
	                            G_SIGNAL_RUN_FIRST,
	                            0,
	                            NULL,
	                            NULL,
	                            g_cclosure_marshal_VOID__BOXED,
	                            G_TYPE_NONE,
	                            1,
                                OSM_TYPE_GPS_MAP_POINT);

    signals [POINT_CHANGED] = g_signal_new ("point-changed",
	                            OSM_TYPE_GPS_MAP_TRACK,
	                            G_SIGNAL_RUN_FIRST,
	                            0,
	                            NULL,
	                            NULL,
	                            g_cclosure_marshal_VOID__VOID,
	                            G_TYPE_NONE,
	                            1,
	                            G_TYPE_INT);

    signals [POINT_INSERTED] = g_signal_new ("point-inserted",
	                            OSM_TYPE_GPS_MAP_TRACK,
	                            G_SIGNAL_RUN_FIRST,
	                            0,
	                            NULL,
	                            NULL,
	                            g_cclosure_marshal_VOID__INT,
	                            G_TYPE_NONE,
	                            1,
	                            G_TYPE_INT);

    signals [POINT_REMOVED] = g_signal_new ("point-removed",
	                            OSM_TYPE_GPS_MAP_TRACK,
	                            G_SIGNAL_RUN_FIRST,
	                            0,
	                            NULL,
	                            NULL,
	                            g_cclosure_marshal_VOID__INT,
	                            G_TYPE_NONE,
	                            1,
	                            G_TYPE_INT);
}

static void
osm_gps_map_track_init (OsmGpsMapTrack *self)
{
    self->priv = G_TYPE_INSTANCE_GET_PRIVATE((self), OSM_TYPE_GPS_MAP_TRACK, OsmGpsMapTrackPrivate);

    self->priv->color.red = DEFAULT_R;
    self->priv->color.green = DEFAULT_G;
    self->priv->color.blue = DEFAULT_B;
}

void
osm_gps_map_track_add_point (OsmGpsMapTrack *track, const OsmGpsMapPoint *point)
{
    g_return_if_fail (OSM_IS_GPS_MAP_TRACK (track));
    OsmGpsMapTrackPrivate *priv = track->priv;

    OsmGpsMapPoint *p = g_boxed_copy (OSM_TYPE_GPS_MAP_POINT, point);
    priv->track = g_slist_append (priv->track, p);
    g_signal_emit (track, signals[POINT_ADDED], 0, p);
}

void
osm_gps_map_track_remove_point(OsmGpsMapTrack* track, int pos)
{
    OsmGpsMapTrackPrivate *priv = track->priv;
    gpointer pgl = g_slist_nth_data(priv->track, pos);
    priv->track = g_slist_remove(priv->track, pgl);
    g_signal_emit(track, signals[POINT_REMOVED], 0, pos);
}

int osm_gps_map_track_n_points(OsmGpsMapTrack* track)
{
    return g_slist_length(track->priv->track);
}

void osm_gps_map_track_insert_point(OsmGpsMapTrack* track, OsmGpsMapPoint* np, int pos)
{
    OsmGpsMapTrackPrivate* priv = track->priv;
    priv->track = g_slist_insert(priv->track, np, pos);
    g_signal_emit(track, signals[POINT_INSERTED], 0, pos);
}

OsmGpsMapPoint* osm_gps_map_track_get_point(OsmGpsMapTrack* track, int pos)
{
    OsmGpsMapTrackPrivate* priv = track->priv;
    return g_slist_nth_data(priv->track, pos);
}


GSList *
osm_gps_map_track_get_points (OsmGpsMapTrack *track)
{
    g_return_val_if_fail (OSM_IS_GPS_MAP_TRACK (track), NULL);
    return track->priv->track;
}

void                
osm_gps_map_track_set_color (OsmGpsMapTrack *track, GdkRGBA *color)
{
    g_return_if_fail (OSM_IS_GPS_MAP_TRACK (track));
    track->priv->color.red = color->red;
    track->priv->color.green = color->green;
    track->priv->color.blue = color->blue;
}

void
osm_gps_map_track_get_color (OsmGpsMapTrack *track, GdkRGBA *color)
{
    g_return_if_fail (OSM_IS_GPS_MAP_TRACK (track));
    color->red = track->priv->color.red;
    color->green = track->priv->color.green;
    color->blue = track->priv->color.blue;
}

double
osm_gps_map_track_get_length(OsmGpsMapTrack* track)
{
    GSList* points = track->priv->track;
    double ret = 0;
    OsmGpsMapPoint* point_a = NULL;
    OsmGpsMapPoint* point_b = NULL;
       
    while(points)
    {
        point_a = point_b;
        point_b = points->data;
        if(point_a)
        {
            ret += acos(sin(point_a->rlat)*sin(point_b->rlat) 
                    + cos(point_a->rlat)*cos(point_b->rlat)*cos(point_b->rlon-point_a->rlon)) * 6371109; //the mean raduis of earth
        }
        points = points->next;
    }
    return ret;
}


OsmGpsMapTrack *
osm_gps_map_track_new (void)
{
    return g_object_new (OSM_TYPE_GPS_MAP_TRACK, NULL);
}

