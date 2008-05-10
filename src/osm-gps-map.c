/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * osm-gps-map.c
 * Copyright (C) John Stowers 2008 <john.stowers@gmail.com>
 * 
 * osm-gps-map.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * osm-gps-map.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gdk/gdk.h>
#include <libsoup/soup.h>

#include "osm-gps-map.h"

typedef struct _OsmGpsMapPrivate OsmGpsMapPrivate;
struct _OsmGpsMapPrivate
{
	SoupSession * soup_session;
	GList * tile_download_list;
	char * cache_dir;
	char * osm_uri;
	
	//TODO: Remove these values which are stored in self, and dont need
	//and independent reference here
	GdkPixmap *pixmap;
	
	int global_drawingarea_width;
	int global_drawingarea_height;
	int global_x;
	int global_y;

	//TODO: These are gobject props, definately need to be renamed prop_x
	int global_zoom;
	gboolean global_autocenter;
	gboolean global_auto_download;

	//For tracking click and drag
	int wtfcounter;
	int mouse_dx;
	int mouse_dy;
	int	mouse_x;
	int	mouse_y;

};

#define OSM_GPS_MAP_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), OSM_TYPE_GPS_MAP, OsmGpsMapPrivate))

typedef struct {
	int x1;
	int y1;
	int x2;
	int y2;
} bbox_pixel_t;

static void
osm_gps_map_fill_tiles_pixel (OsmGpsMap *map, int pixel_x, int pixel_y, int zoom);

enum
{
	PROP_0,

	PROP_AUTO_CENTER,
	PROP_AUTO_DOWNLOAD,
	PROP_REPO_URI,
	PROP_TILE_CACHE,
	PROP_ZOOM,
	PROP_INVERT_ZOOM
};

gboolean
osm_gps_map_scroll (GtkWidget *widget, GdkEventScroll  *event)
{
	if ((event->state & GDK_CONTROL_MASK) == GDK_CONTROL_MASK) {
		if (event->direction == GDK_SCROLL_UP) {
			g_printf("SCROLL UP+Ctrl\n");
		} else {
			g_printf("SCROLL DOWN+Ctrl\n");
		}
	}
	else
	{
		if (event->direction == GDK_SCROLL_UP)
		{
			g_printf("SCROLL UP\n");
			//TODO: Zoom in
			//on_button4_clicked(NULL,NULL);
		}
		else
		{
			g_printf("SCROLL DOWN\n");
			//TODO: Zoom out
			//on_button5_clicked(NULL,NULL);

		}
	}
	return FALSE;
}

gboolean
osm_gps_map_button_press (GtkWidget *widget, GdkEventButton *event)
{
	/* TODO: Add private function implementation here */
}

gboolean
osm_gps_map_button_release (GtkWidget *widget, GdkEventButton *event)
{
	/* TODO: Add private function implementation here */
}

gboolean
osm_gps_map_motion_notify (GtkWidget *widget, GdkEventMotion  *event)
{
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(widget);
	
	//g_printf("motion event\n");

	int x, y, width, height;
	GdkModifierType state;
	
	width  = widget->allocation.width;//only to repaint the whole thing
	height = widget->allocation.height;//FIXME: better dont do this?
	
	if (event->is_hint)
		gdk_window_get_pointer (event->window, &x, &y, &state);
	else
	{
		x = event->x;
		y = event->y;
		state = event->state;
	}
 
	if ( (state & GDK_BUTTON1_MASK)  && (priv->wtfcounter >= 6) ) {
		priv->global_autocenter = FALSE;
		priv->mouse_dx = x - priv->mouse_x;	
		priv->mouse_dy = y - priv->mouse_y;
		gdk_draw_drawable (
			widget->window,
			widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
			priv->pixmap,
			0,0,
			priv->mouse_dx,priv->mouse_dy,
			-1,-1);

		//g_printf("motion: %i %i - start: %i %i - dx: %i %i --wtf %i\n", x,y, mouse_x, mouse_y, mouse_dx, mouse_dy,wtfcounter);
	}	
	else
		priv->wtfcounter++;

	return FALSE;
}

gboolean
osm_gps_map_configure (GtkWidget *widget, GdkEventConfigure *event)
{
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(widget);
	
	priv->global_drawingarea_width  = widget->allocation.width;
	priv->global_drawingarea_height = widget->allocation.height;

	/* create pixmap */
	if (priv->pixmap)
		g_object_unref (priv->pixmap);

	priv->pixmap = gdk_pixmap_new (
			widget->window,
			widget->allocation.width+260, //TODO: this could be cleverer
			widget->allocation.height+260,
			-1);
//TODOremove or not...
//#if 0
//if(pixmap) printf("pixmap NOT NULL");
//else printf("aieee: pixmap NULL\n");

	/* draw white background to initialise pixmap */
	gdk_draw_rectangle (
		priv->pixmap,
		widget->style->white_gc,
		TRUE,
		0, 0,
		widget->allocation.width+260,
		widget->allocation.height+260);
				
	gtk_widget_queue_draw_area (
		widget, 
		0,0,widget->allocation.width+260,widget->allocation.height+260);
//#endif

	//todo: is this double call on init()?
	//needed for resize event
	osm_gps_map_fill_tiles_pixel(OSM_GPS_MAP(widget),
								 priv->global_x,
								 priv->global_y,
								 priv->global_zoom);
	
	return FALSE;
}

gboolean
osm_gps_map_expose (GtkWidget *widget, GdkEventExpose  *event)
{
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(widget);

	gdk_draw_drawable (
		widget->window,
		widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
		priv->pixmap,
		event->area.x, event->area.y,
		event->area.x, event->area.y,
		event->area.width, event->area.height);
	
	return FALSE;
}

static void
osm_gps_map_tile_download_complete (SoupMessage *req, gpointer user_data)
{
	/* TODO: Add private function implementation here */
}

static void
osm_gps_map_queue_tile_dl_for_bbox (OsmGpsMap *map, bbox_pixel_t bbox_pixel, int zoom)
{
	/* TODO: Add private function implementation here */
}

static bbox_pixel_t
osm_gps_map_get_bbox_pixel (OsmGpsMap *map, bbox_t bbox, int zoom)
{
	/* TODO: Add private function implementation here */
}

static gboolean
osm_gps_map_timer_tile_download (gpointer data)
{
	/* TODO: Add private function implementation here */
}

static void
osm_gps_map_load_tile (OsmGpsMap *map, gchar *dir, int zoom, int x, int y, int offset_x, int offset_y)
{
	/* TODO: Add private function implementation here */
}

static void
osm_gps_map_fill_tiles_pixel (OsmGpsMap *map, int pixel_x, int pixel_y, int zoom)
{
	/* TODO: Add private function implementation here */
}

static void
osm_gps_map_fill_tiles_latlon (OsmGpsMap *map, float lat, float lon, int zoom)
{
	/* TODO: Add private function implementation here */
}

G_DEFINE_TYPE (OsmGpsMap, osm_gps_map, GTK_TYPE_DRAWING_AREA);

static void
osm_gps_map_init (OsmGpsMap *object)
{
	/* TODO: Add initialization code here */
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(object);
	
	priv->pixmap = NULL;
	priv->global_drawingarea_width = 0;
	priv->global_drawingarea_height = 0;
	priv->global_x = 890;
	priv->global_y = 515;

	priv->wtfcounter = 0;
	priv->mouse_dx = 0;
	priv->mouse_dy = 0;
	priv->mouse_x = 0;
	priv->mouse_y = 0;
	
	//TODO: These should get set by gobject props
	priv->global_autocenter = TRUE;
	priv->global_auto_download = TRUE;
	priv->global_zoom = 3;
	
	gtk_widget_add_events (GTK_WIDGET (object),
			GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
}

static void
osm_gps_map_finalize (GObject *object)
{
	/* TODO: Add deinitalization code here */

	G_OBJECT_CLASS (osm_gps_map_parent_class)->finalize (object);
}

static void
osm_gps_map_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (OSM_IS_GPS_MAP (object));

	switch (prop_id)
	{
	case PROP_AUTO_CENTER:
		/* TODO: Add setter for "auto-center" property here */
		break;
	case PROP_AUTO_DOWNLOAD:
		/* TODO: Add setter for "auto-download" property here */
		break;
	case PROP_REPO_URI:
		/* TODO: Add setter for "repo-uri" property here */
		break;
	case PROP_TILE_CACHE:
		/* TODO: Add setter for "tile-cache" property here */
		break;
	case PROP_ZOOM:
		/* TODO: Add setter for "zoom" property here */
		break;
	case PROP_INVERT_ZOOM:
		/* TODO: Add setter for "invert-zoom" property here */
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
osm_gps_map_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (OSM_IS_GPS_MAP (object));

	switch (prop_id)
	{
	case PROP_AUTO_CENTER:
		/* TODO: Add getter for "auto-center" property here */
		break;
	case PROP_AUTO_DOWNLOAD:
		/* TODO: Add getter for "auto-download" property here */
		break;
	case PROP_REPO_URI:
		/* TODO: Add getter for "repo-uri" property here */
		break;
	case PROP_TILE_CACHE:
		/* TODO: Add getter for "tile-cache" property here */
		break;
	case PROP_ZOOM:
		/* TODO: Add getter for "zoom" property here */
		break;
	case PROP_INVERT_ZOOM:
		/* TODO: Add getter for "invert-zoom" property here */
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
osm_gps_map_class_init (OsmGpsMapClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	GtkDrawingAreaClass* parent_class = GTK_DRAWING_AREA_CLASS (klass);

	g_type_class_add_private (klass, sizeof (OsmGpsMapPrivate));

	object_class->finalize = osm_gps_map_finalize;
	object_class->set_property = osm_gps_map_set_property;
	object_class->get_property = osm_gps_map_get_property;

	g_object_class_install_property (object_class,
	                                 PROP_AUTO_CENTER,
	                                 g_param_spec_boolean ("auto-center",
	                                                       "auto center",
	                                                       "map auto center",
	                                                       TRUE,
	                                                       G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));

	g_object_class_install_property (object_class,
	                                 PROP_AUTO_DOWNLOAD,
	                                 g_param_spec_boolean ("auto-download",
	                                                       "auto download",
	                                                       "map auto download",
	                                                       TRUE,
	                                                       G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));

	g_object_class_install_property (object_class,
	                                 PROP_REPO_URI,
	                                 g_param_spec_string ("repo-uri",
	                                                      "repo uri",
	                                                      "osm repo uri",
	                                                      "http://foo",
	                                                      G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));

	g_object_class_install_property (object_class,
	                                 PROP_TILE_CACHE,
	                                 g_param_spec_string ("tile-cache",
	                                                      "tile cache",
	                                                      "osm local tile cache dir",
	                                                      "/tmp/OSM/cache",
	                                                      G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));

	g_object_class_install_property (object_class,
	                                 PROP_ZOOM,
	                                 g_param_spec_uint ("zoom",
	                                                    "zoom",
	                                                    "zoome level",
	                                                    0, /* TODO: Adjust minimum property value */
	                                                    G_MAXUINT, /* TODO: Adjust maximum property value */
	                                                    3,
	                                                    G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));

	g_object_class_install_property (object_class,
	                                 PROP_INVERT_ZOOM,
	                                 g_param_spec_boolean ("invert-zoom",
	                                                       "invert zoom",
	                                                       "is zoom inverted",
	                                                       FALSE,
	                                                       G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));
}


void
osm_gps_map_download_maps (OsmGpsMap *map, bbox_t bbox, int zoom_start, int zoom_end)
{
	/* TODO: Add public function implementation here */
}

void
osm_gps_map_download_tile (OsmGpsMap *map, int zoom, int x, int y)
{
	/* TODO: Add public function implementation here */
}

bbox_t
osm_gps_map_get_bbox (OsmGpsMap *map)
{
	/* TODO: Add public function implementation here */
	bbox_t b;
	return b;
}

void
osm_gps_map_map_redraw (OsmGpsMap *map)
{
	/* TODO: Add public function implementation here */
}

void
osm_gps_map_set_mapcenter (OsmGpsMap *map, float lat, float lon)
{
	/* TODO: Add public function implementation here */
}

void
osm_gps_map_print_track (OsmGpsMap *map, GList *trackpoint_list)
{
	/* TODO: Add public function implementation here */
}

void
osm_gps_map_paint_image (OsmGpsMap *map, float lat, float lon, GdkPixbuf *image, int w, int h)
{
	/* TODO: Add public function implementation here */
}

tile_t
osm_gps_map_get_tile (OsmGpsMap *map, int pixel_x, int pixel_y, int zoom)
{
	/* TODO: Add public function implementation here */
	tile_t t;
	return t;
}

void
osm_gps_map_osd_speed (OsmGpsMap *map, float speed)
{
	/* TODO: Add public function implementation here */
}

void
osm_gps_map_draw_gps (OsmGpsMap *map, float lat, float lon)
{
	/* TODO: Add public function implementation here */
}
