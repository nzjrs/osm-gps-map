/*
 * Copyright (C) 2013 Samuel Cowen <samuel.cowen@camelsoftware.com>
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


#include <gdk/gdk.h>

#include "converter.h"
#include "osm-gps-map-polygon.h"

G_DEFINE_TYPE (OsmGpsMapPolygon, osm_gps_map_polygon, G_TYPE_OBJECT)

enum
{
    PROP_0,
    PROP_VISIBLE,
    PROP_TRACK,
	PROP_SHADED,
    PROP_EDITABLE,
    PROP_SHADE_ALPHA,
    PROP_BREAKABLE
};

struct _OsmGpsMapPolygonPrivate
{
    OsmGpsMapTrack* track;
	gboolean visible;
    gboolean editable;
	gboolean shaded;
    gfloat shade_alpha;
    gboolean breakable;
};

#define DEFAULT_R   (60000)
#define DEFAULT_G   (0)
#define DEFAULT_B   (0)
#define DEFAULT_A   (0.6)

static void
osm_gps_map_polygon_get_property (GObject    *object,
                                guint       property_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
    OsmGpsMapPolygonPrivate *priv = OSM_GPS_MAP_POLYGON(object)->priv;

    switch (property_id)
    {
        case PROP_VISIBLE:
            g_value_set_boolean(value, priv->visible);
            break;
        case PROP_TRACK:
            g_value_set_pointer(value, priv->track);
            break;
		case PROP_SHADED:
			g_value_set_boolean(value, priv->shaded);
			break;
        case PROP_EDITABLE:
            g_value_set_boolean(value, priv->editable);
            break;
        case PROP_SHADE_ALPHA:
            g_value_set_float(value, priv->shade_alpha);
            break;
        case PROP_BREAKABLE:
            g_value_set_boolean(value, priv->breakable);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
osm_gps_map_polygon_set_property (GObject      *object,
                                guint         property_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
    OsmGpsMapPolygonPrivate *priv = OSM_GPS_MAP_POLYGON(object)->priv;

    switch (property_id)
    {
        case PROP_VISIBLE:
            priv->visible = g_value_get_boolean (value);
            break;
        case PROP_TRACK:
            priv->track = g_value_get_pointer (value);
            break;
        case PROP_SHADED:
			priv->shaded = g_value_get_boolean(value);
			break;
        case PROP_EDITABLE:
            priv->editable = g_value_get_boolean(value);
            break;
        case PROP_SHADE_ALPHA:
            priv->shade_alpha = g_value_get_float(value);
            break;
        case PROP_BREAKABLE:
            priv->breakable = g_value_get_boolean(value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
    }
}

static void
osm_gps_map_polygon_dispose (GObject *object)
{
    g_return_if_fail (OSM_IS_GPS_MAP_POLYGON (object));
	OsmGpsMapPolygon* poly = OSM_GPS_MAP_POLYGON(object);
	g_object_unref(poly->priv->track);

    G_OBJECT_CLASS (osm_gps_map_polygon_parent_class)->dispose (object);
}

static void
osm_gps_map_polygon_finalize (GObject *object)
{
    G_OBJECT_CLASS (osm_gps_map_polygon_parent_class)->finalize (object);
}

static void
osm_gps_map_polygon_class_init (OsmGpsMapPolygonClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);

    g_type_class_add_private (klass, sizeof (OsmGpsMapPolygonPrivate));

    object_class->get_property = osm_gps_map_polygon_get_property;
    object_class->set_property = osm_gps_map_polygon_set_property;
    object_class->dispose = osm_gps_map_polygon_dispose;
    object_class->finalize = osm_gps_map_polygon_finalize;

    g_object_class_install_property (object_class,
                                     PROP_VISIBLE,
                                     g_param_spec_boolean ("visible",
                                                           "visible",
                                                           "should this poly be visible",
                                                           TRUE,
                                                           G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (object_class,
                                     PROP_TRACK,
                                     g_param_spec_pointer ("track",
                                                           "track",
                                                           "list of points for the polygon",
                                                           G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));


    g_object_class_install_property (object_class,
                                     PROP_EDITABLE,
                                     g_param_spec_boolean ("editable",
                                                           "editable",
                                                           "should this polygon be editable",
                                                           FALSE,
                                                           G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));

	g_object_class_install_property (object_class,
                                     PROP_SHADED,
                                     g_param_spec_boolean ("shaded",
                                                           "shaded",
                                                           "should this polygon be shaded",
                                                           TRUE,
                                                           G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));

    g_object_class_install_property (object_class,
                                     PROP_SHADE_ALPHA,
                                    g_param_spec_float ("shade_alpha",
                                                        "shade_alpha",
                                                        "sets the translucency of the shaded area of a polygon",
                                                        0.0,
                                                        1.0,
                                                        0.5,
                                                        G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));

    g_object_class_install_property(object_class,
                                    PROP_BREAKABLE,
                                    g_param_spec_boolean("breakable",
                                                        "breakable",
                                                        "can polygons have points inserted using breakers",
                                                        TRUE,
                                                        G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));

}

static void
osm_gps_map_polygon_init (OsmGpsMapPolygon *self)
{
    self->priv = G_TYPE_INSTANCE_GET_PRIVATE((self), OSM_TYPE_GPS_MAP_POLYGON, OsmGpsMapPolygonPrivate);
	self->priv->track = osm_gps_map_track_new();
}

OsmGpsMapTrack*
osm_gps_map_polygon_get_track(OsmGpsMapPolygon* poly)
{
	return poly->priv->track;
}

OsmGpsMapPolygon *
osm_gps_map_polygon_new (void)
{
    return g_object_new (OSM_TYPE_GPS_MAP_POLYGON, "track", osm_gps_map_track_new(), NULL);
}

#ifdef __cplusplus
}
#endif
