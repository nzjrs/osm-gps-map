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

enum
{
    PROP_0,
    PROP_VISIBLE,
    PROP_TRACK,
    PROP_LINE_WIDTH,
    PROP_ALPHA, /* The alpha property can be considered obsolete as GdkRGBA also provides an alpha field */
    PROP_COLOR,
    PROP_EDITABLE,
    PROP_CLICKABLE,
    PROP_HIGHLIGHT_POINT,
    PROP_HIGHLIGHT_COLOR
};

enum
{
    POINT_ADDED,
    POINT_CHANGED,
    POINT_INSERTED,
    POINT_REMOVED,
    POINT_CLICKED,
    LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = {0,};

struct _OsmGpsMapTrackPrivate
{
    GSList *track;
    gboolean visible;
    gfloat linewidth;
    GdkRGBA color;
    gboolean editable;
    gboolean clickable;

    /*
     * Properties for highlighted points.
     * highlight_point - Must be a pointer to a element in the list of points
     * associated with this track or NULL.
     * highlight_color - The color used to indicate this point as highlighted.
     */
    OsmGpsMapPoint *highlight_point;
    GdkRGBA highlight_color;
};

G_DEFINE_TYPE_WITH_PRIVATE(OsmGpsMapTrack, osm_gps_map_track, G_TYPE_OBJECT)

#define DEFAULT_R   (0.6)
#define DEFAULT_G   (0)
#define DEFAULT_B   (0)
#define DEFAULT_A   (0.6)

#define DEFAULT_HIGHLIGHT_R   (0)
#define DEFAULT_HIGHLIGHT_G   (0)
#define DEFAULT_HIGHLIGHT_B   (0.6)
#define DEFAULT_HIGHLIGHT_A   (0.6)

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
            g_value_set_float(value, priv->color.alpha);
            break;
        case PROP_COLOR:
            g_value_set_boxed(value, &priv->color);
            break;
        case PROP_EDITABLE:
            g_value_set_boolean(value, priv->editable);
            break;
        case PROP_CLICKABLE:
            g_value_set_boolean(value, priv->clickable);
            break;
        case PROP_HIGHLIGHT_POINT:
            g_value_set_pointer(value, priv->highlight_point);
            break;
        case PROP_HIGHLIGHT_COLOR:
            g_value_set_boxed(value, &priv->highlight_color);
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
            priv->color.alpha = g_value_get_float (value);
            break;
        case PROP_COLOR: {
            GdkRGBA *c = g_value_get_boxed (value);
            priv->color.red = c->red;
            priv->color.green = c->green;
            priv->color.blue = c->blue;
            priv->color.alpha = c->alpha;
            } break;
        case PROP_EDITABLE:
            priv->editable = g_value_get_boolean(value);
            break;
        case PROP_CLICKABLE:
            priv->clickable = g_value_get_boolean(value);
            break;
        case PROP_HIGHLIGHT_POINT:
            priv->highlight_point = g_value_get_pointer(value);
            break;
        case PROP_HIGHLIGHT_COLOR: {
            GdkRGBA *c = g_value_get_boxed (value);
            priv->highlight_color.red = c->red;
            priv->highlight_color.green = c->green;
            priv->highlight_color.blue = c->blue;
            priv->highlight_color.alpha = c->alpha;
            } break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
osm_gps_map_track_dispose (GObject *object)
{
    g_return_if_fail (OSM_GPS_MAP_IS_TRACK (object));
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
                                                         G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT | G_PARAM_DEPRECATED));

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

    g_object_class_install_property (object_class,
                                     PROP_CLICKABLE,
                                     g_param_spec_boolean ("clickable",
                                                           "clickable",
                                                           "should this track be clickable",
                                                           FALSE,
                                                           G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (object_class,
                                     PROP_HIGHLIGHT_POINT,
                                     g_param_spec_pointer ("highlight-point",
                                                           "highlight point",
                                                           "point in this track that must be highlighted",
                                                           G_PARAM_READABLE | G_PARAM_WRITABLE));

    g_object_class_install_property (object_class,
                                     PROP_HIGHLIGHT_COLOR,
                                     g_param_spec_boxed ("highlight-color",
                                                         "highlight color",
                                                         "color used to mark a highlighted point",
                                                         GDK_TYPE_RGBA,
                                                         G_PARAM_READABLE | G_PARAM_WRITABLE));

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
	                            g_cclosure_marshal_VOID__INT,
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

    /*
     * This signal is emitted when a point on the map is clicked/selected. The
     * callback(s) connected to this signal will receive a pointer to the point
     * which was clicked (hence the G_SIGNAL_TYPE_STATIC_SCOPE flag).
     */
    signals [POINT_CLICKED] = g_signal_new ("point-clicked",
	                            OSM_TYPE_GPS_MAP_TRACK,
	                            G_SIGNAL_RUN_FIRST,
	                            0,
	                            NULL,
	                            NULL,
	                            g_cclosure_marshal_VOID__POINTER,
	                            G_TYPE_NONE,
	                            1,
	                            G_TYPE_POINTER | G_SIGNAL_TYPE_STATIC_SCOPE);
}

static void
osm_gps_map_track_init (OsmGpsMapTrack *self)
{
    self->priv = osm_gps_map_track_get_instance_private(self);

    self->priv->color.red = DEFAULT_R;
    self->priv->color.green = DEFAULT_G;
    self->priv->color.blue = DEFAULT_B;
    self->priv->color.alpha = DEFAULT_A;

    self->priv->highlight_color.red = DEFAULT_HIGHLIGHT_R;
    self->priv->highlight_color.green = DEFAULT_HIGHLIGHT_G;
    self->priv->highlight_color.blue = DEFAULT_HIGHLIGHT_B;
    self->priv->highlight_color.alpha = DEFAULT_HIGHLIGHT_A;

    self->priv->highlight_point = NULL;
}

void
osm_gps_map_track_add_point (OsmGpsMapTrack *track, const OsmGpsMapPoint *point)
{
    g_return_if_fail (OSM_GPS_MAP_IS_TRACK (track));
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
    g_return_val_if_fail (OSM_GPS_MAP_IS_TRACK (track), NULL);
    return track->priv->track;
}

void
osm_gps_map_track_set_color (OsmGpsMapTrack *track, GdkRGBA *color)
{
    g_return_if_fail (OSM_GPS_MAP_IS_TRACK (track));
    track->priv->color.red = color->red;
    track->priv->color.green = color->green;
    track->priv->color.blue = color->blue;
    track->priv->color.alpha = color->alpha;
}

void
osm_gps_map_track_get_color (OsmGpsMapTrack *track, GdkRGBA *color)
{
    g_return_if_fail (OSM_GPS_MAP_IS_TRACK (track));
    color->red = track->priv->color.red;
    color->green = track->priv->color.green;
    color->blue = track->priv->color.blue;
    color->alpha = track->priv->color.alpha;
}

void
osm_gps_map_track_set_highlight_color (OsmGpsMapTrack *track, GdkRGBA *color)
{
    g_return_if_fail (OSM_GPS_MAP_IS_TRACK (track));
    track->priv->highlight_color.red = color->red;
    track->priv->highlight_color.green = color->green;
    track->priv->highlight_color.blue = color->blue;
    track->priv->highlight_color.alpha = color->alpha;
}

void
osm_gps_map_track_get_highlight_color (OsmGpsMapTrack *track, GdkRGBA *color)
{
    g_return_if_fail (OSM_GPS_MAP_IS_TRACK (track));
    color->red = track->priv->highlight_color.red;
    color->green = track->priv->highlight_color.green;
    color->blue = track->priv->highlight_color.blue;
    color->alpha = track->priv->highlight_color.alpha;
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

void
osm_gps_map_track_set_highlight_point(OsmGpsMapTrack* track, OsmGpsMapPoint *point)
{
    g_return_if_fail (OSM_GPS_MAP_IS_TRACK (track));
    track->priv->highlight_point = point;
    /* Notify ourself of the new highlight point.
     * When a map object is listening it will update the map. */
    g_object_notify (G_OBJECT (track), "highlight-point");
}

OsmGpsMapPoint *
osm_gps_map_track_get_highlight_point(OsmGpsMapTrack* track)
{
    g_return_val_if_fail (OSM_GPS_MAP_IS_TRACK (track), NULL);
    return track->priv->highlight_point;
}


OsmGpsMapTrack *
osm_gps_map_track_new (void)
{
    return g_object_new (OSM_TYPE_GPS_MAP_TRACK, NULL);
}

