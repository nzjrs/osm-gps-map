#include "osm-gps-map-layer.h"

GType osm_gps_map_layer_get_type()
{
	static GType object_type = 0;
	if (!object_type) {
		static const GTypeInfo object_info = {
			sizeof(OsmGpsMapLayerIface),
			NULL,	/* base init */
			NULL,	/* base finalize */
		};
		object_type =
		    g_type_register_static(G_TYPE_INTERFACE,
					   "OsmGpsMapLayer",
					   &object_info, 0);
	}
	return object_type;
}

void
osm_gps_map_layer_render (OsmGpsMapLayer *self, OsmGpsMap *map)
{
	OSM_GPS_MAP_LAYER_GET_INTERFACE (self)->render (self, map);
}

void
osm_gps_map_layer_draw (OsmGpsMapLayer *self, OsmGpsMap *map, GdkDrawable *drawable)
{
	OSM_GPS_MAP_LAYER_GET_INTERFACE (self)->draw (self, map, drawable);
}

gboolean
osm_gps_map_layer_busy (OsmGpsMapLayer *self)
{
	return OSM_GPS_MAP_LAYER_GET_INTERFACE (self)->busy (self);
}

gboolean
osm_gps_map_layer_button_press (OsmGpsMapLayer *self, OsmGpsMap *map, GdkEventButton *event)
{
	return OSM_GPS_MAP_LAYER_GET_INTERFACE (self)->button_press (self, map, event);
}

