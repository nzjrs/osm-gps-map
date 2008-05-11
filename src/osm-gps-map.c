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
//#include <stdio.h>
#include <fcntl.h>
#include <math.h>

#include <gdk/gdk.h>
#include <glib/gprintf.h>
#include <libsoup/soup.h>

#include "converter.h"
#include "osm-gps-map-types.h"
#include "osm-gps-map.h"

typedef struct _OsmGpsMapPrivate OsmGpsMapPrivate;
struct _OsmGpsMapPrivate
{
	//TODO: These are gobject props, definately need to be renamed prop_x
	SoupSession * soup_session;
	char * cache_dir;
	char * repo_uri;
	gboolean invert_zoom;
	int global_zoom;
	gboolean global_autocenter;
	gboolean global_auto_download;
	
	//TODO: Remove these values which are stored in self, and dont need
	//and independent reference here
	GdkPixmap *pixmap;
	GdkGC *gc_map;
	
	int global_drawingarea_width;
	int global_drawingarea_height;
	int global_x;
	int global_y;

	//For tracking click and drag
	int wtfcounter;
	int mouse_dx;
	int mouse_dy;
	int	mouse_x;
	int	mouse_y;
	int local_x;
	int local_y;
};

#define OSM_GPS_MAP_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), OSM_TYPE_GPS_MAP, OsmGpsMapPrivate))

typedef struct {
	/* The details of the tile to download */
	char *uri;
	char *folder;
	char *filename;
	/* The area on the screen to redraw when it arrives */
	OsmGpsMap *map;
	int offset_x;
	int offset_y;
} tile_download_t;

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
	OsmGpsMap *map = OSM_GPS_MAP(widget);
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(widget);

	if ((event->state & GDK_CONTROL_MASK) == GDK_CONTROL_MASK) {
		if (event->direction == GDK_SCROLL_UP) {
			g_debug("SCROLL UP+Ctrl");
		} else {
			g_debug("SCROLL DOWN+Ctrl");
		}
	}
	else
	{
		if (event->direction == GDK_SCROLL_UP)
		{
			g_debug("SCROLL UP");
			osm_gps_map_set_zoom(map, priv->global_zoom+1);
		}
		else
		{
			g_debug("SCROLL DOWN");
			osm_gps_map_set_zoom(map, priv->global_zoom-1);
		}
	}
	return FALSE;
}

gboolean
osm_gps_map_button_press (GtkWidget *widget, GdkEventButton *event)
{
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(widget);

	priv->wtfcounter = 0;
	if ( (event->type==GDK_BUTTON_PRESS || event->type==GDK_2BUTTON_PRESS) )
	{
		g_debug("%s clicked with button %d",event->type==GDK_BUTTON_PRESS ? "single" : "double", event->button);
		//TODO:needs timer or kill popup
		//if(event->type==GDK_2BUTTON_PRESS)
		//	on_button4_clicked(NULL,NULL);
			
	}

	priv->mouse_x = (int) event->x;
	priv->mouse_y = (int) event->y;
	priv->local_x = priv->global_x;
	priv->local_y = priv->global_y;
	/*
	global_x += (int)event->x;
	global_y += (int)event->y;

	fill_tiles_pixel(global_x, global_y, global_zoom);
	*/
	//if (event->button == 1 && pixmap != NULL)
		//draw_circle (widget, event->x, event->y);

	//printf("--- %s() %d %d: \n",__PRETTY_FUNCTION__, global_x, local_x);
	
	return FALSE;
}

gboolean
osm_gps_map_button_release (GtkWidget *widget, GdkEventButton *event)
{
	GtkMenu *menu;
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(widget);

	//printf("*** %s() %d %d: \n",__PRETTY_FUNCTION__, global_x, local_x);
	
	//if(global_mapmode)
	if(priv->wtfcounter >= 6)
	{
		g_debug("mouse drag +8events");
		//int mouse_dx, mouse_dy;
		
		priv->global_x = priv->local_x;//FIXME unnecessary oder auch nicht: kein redraw, wenn nicht bewegt
		priv->global_y = priv->local_y;
		
		//mouse_dx = mouse_x - (int) event->x;//FIXME entweder dx oder mouse_dx
		//mouse_dy = mouse_y - (int) event->y;
		
		priv->global_x += (priv->mouse_x - (int) event->x);
		priv->global_y += (priv->mouse_y - (int) event->y);
	
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
		
		osm_gps_map_fill_tiles_pixel(OSM_GPS_MAP(widget),priv->global_x, priv->global_y, priv->global_zoom);	//FIXME zoom
		
	
		//print_track();
		//paint_friends();
		//paint_photos();
		//paint_pois();
		//printf("mouse delta: %d %d\n", mouse_dx, mouse_dy);
	}
	else
	{
		//menu = GTK_MENU(create_menu1()); //GTK_MENU(lookup_widget(window1,"menu1"));
		//gtk_menu_popup (menu, NULL, NULL, NULL, NULL, event->button, event->time);
		g_debug("Popup menu");
	}

	/* ambiguity: this is global mouse dx,y */	
	priv->mouse_dx = 0;
	priv->mouse_dy = 0;
	priv->wtfcounter = 0;

    //printf("--- %s() %d %d: \n",__PRETTY_FUNCTION__, global_x, local_x);
	return FALSE;

}

gboolean
osm_gps_map_motion_notify (GtkWidget *widget, GdkEventMotion  *event)
{
	int x, y, width, height;
	GdkModifierType state;
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(widget);
	
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

		g_debug("motion: %i %i - start: %i %i - dx: %i %i --wtf %i\n", x,y, priv->mouse_x, priv->mouse_y, priv->mouse_dx, priv->mouse_dy, priv->wtfcounter);
	}	
	else
		priv->wtfcounter++;

	return FALSE;
}

gboolean
osm_gps_map_configure (GtkWidget *widget, GdkEventConfigure *event)
{
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(widget);

	g_debug("CONFIGURE");	

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

	g_debug("EXPOSE");

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
osm_gps_map_blit_tile(OsmGpsMap *map, GdkPixbuf *pixbuf, int offset_x, int offset_y)
{
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);	
	
	g_debug("Queing redraw @ %d,%d (w:%d h:%d)", offset_x,offset_y, TILESIZE,TILESIZE);
	
	//TODO: Init all these things on first expose/configure??
	if(priv->gc_map)
		g_object_unref(priv->gc_map);

	if(priv->pixmap)
		priv->gc_map = gdk_gc_new(priv->pixmap);
	else
		g_warning("no drawable -> NULL\n");
	
	/* draw pixbuf onto pixmap */
	gdk_draw_pixbuf (priv->pixmap,
					 priv->gc_map,//TODO: It works if I just pass NULL here, should i?
					 pixbuf,
					 0,0,
					 offset_x,offset_y,
					 TILESIZE,TILESIZE,
					 GDK_RGB_DITHER_NONE, 0, 0);

	gtk_widget_queue_draw_area (GTK_WIDGET(map),
								offset_x,offset_y,
								TILESIZE,TILESIZE);
}

static void
osm_gps_map_tile_download_complete (SoupSession *session, SoupMessage *msg, gpointer user_data)
{
	int fd;
	tile_download_t *dl = (tile_download_t *)user_data;
	
	if (!SOUP_STATUS_IS_SUCCESSFUL (msg->status_code)) {
		g_warning("Error downloading tile: %d %s", msg->status_code, msg->reason_phrase);
		return;
	}

	if (g_mkdir_with_parents(dl->folder,0700) != 0) {
		g_warning("Error creating tile download directory: %s", dl->folder);
		return;
	}

	fd = g_open(dl->filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd != -1) {
		write (fd, msg->response_body->data, msg->response_body->length);
		g_debug("Wrote %lld bytes to %s", msg->response_body->length, dl->filename);
		close (fd);
		
		/* Redraw the area of the screen */
		if ( TRUE ) {
			GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file (dl->filename, NULL);
			if(pixbuf) {
				g_debug("Found tile %s", dl->filename);
				osm_gps_map_blit_tile(dl->map, pixbuf, dl->offset_x,dl->offset_y);
				g_object_unref (pixbuf);
			}
		}
	}
			
	g_free(dl->uri);
	g_free(dl->folder);
	g_free(dl->filename);
	g_free(dl);
}

static void
osm_gps_map_queue_tile_dl_for_bbox (OsmGpsMap *map, bbox_pixel_t bbox_pixel, int zoom)
{
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);
	tile_t tile_11, tile_22;
	int i,j,k=0;
	
	g_debug("Queuing tile/s for download");

	tile_11 = osm_gps_map_get_tile(map, bbox_pixel.x1, bbox_pixel.y1, zoom);
	tile_22 = osm_gps_map_get_tile(map, bbox_pixel.x2, bbox_pixel.y2, zoom);
	
	// loop x1-x2
	for(i=tile_11.x; i<=tile_22.x; i++)
	{
		// loop y1 - y2
		for(j=tile_11.y; j<=tile_22.y; j++)
		{
			// x = i, y = j
			osm_gps_map_download_tile(map, zoom, i, j, -1, -1);
		}
	}
}

static bbox_pixel_t
osm_gps_map_get_bbox_pixel (OsmGpsMap *map, bbox_t bbox, int zoom)
{
	bbox_pixel_t bbox_pixel;
	
	bbox_pixel.x1 = lon2pixel(zoom, bbox.lon1);
	bbox_pixel.y1 = lat2pixel(zoom, bbox.lat1);
	bbox_pixel.x2 = lon2pixel(zoom, bbox.lon2);
	bbox_pixel.y2 = lat2pixel(zoom, bbox.lat2);

	g_debug("1:%d,%d 2:%d,%d",bbox_pixel.x1,bbox_pixel.y1,bbox_pixel.x2,bbox_pixel.y2);
	
	return	bbox_pixel;
}

static gboolean
osm_gps_map_timer_tile_download (gpointer data)
{
	/* TODO: Add private function implementation here */
	//TODO: NO LONGER NEEDED IF SOUP LIMITS THE NUMBER OF DL THREADS ANYWAY
}

static void
osm_gps_map_load_tile (OsmGpsMap *map, int zoom, int x, int y, int offset_x, int offset_y)
{
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);
	gchar *filename;
	GdkPixbuf *pixbuf;

	g_debug("Load tile %d,%d (%d,%d) z:%d", x, y, offset_x, offset_y, zoom);

	filename = g_strdup_printf("%s/%u/%u/%u.png", 
							   priv->cache_dir,
							   zoom, x, y);

	pixbuf = gdk_pixbuf_new_from_file (filename, NULL);
	if(pixbuf) 
	{
		g_debug("Found tile %s", filename);
		osm_gps_map_blit_tile(map, pixbuf, offset_x,offset_y);
		g_object_unref (pixbuf);
	}
	else
	{
		if (priv->global_auto_download)
			osm_gps_map_download_tile(map,zoom,x,y,offset_x,offset_y);
	}
	g_free(filename);
}

static void
osm_gps_map_fill_tiles_pixel (OsmGpsMap *map, int pixel_x, int pixel_y, int zoom)
{
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);
	int i,j, width, height, tile_x0, tile_y0, tiles_nx, tiles_ny;
	int offset_xn = 0;
	int offset_yn = 0;
	int offset_x;
	int offset_y;
	
	g_debug("Fill tiles: %d,%d z:%d", pixel_x, pixel_y, zoom);
	
	//offset_x = -fmod(pixel_x , TILESIZE);//was I on the moon?
	offset_x = - pixel_x % TILESIZE;
	offset_y = - pixel_y % TILESIZE;
	if (offset_x > 0) offset_x -= 256;
	if (offset_y > 0) offset_y -= 256;
	
	priv->global_x = pixel_x;
	priv->global_y = pixel_y;
	priv->global_zoom = zoom;
	
	offset_xn = offset_x; //FIXME this is gedoppelt
	offset_yn = offset_y;
	// w/h drawable
	width  = GTK_WIDGET(map)->allocation.width;
	height = GTK_WIDGET(map)->allocation.height;

	tiles_nx = floor((width  - offset_x) / TILESIZE) + 1;//TODO right # of tiles 0-1
	tiles_ny = floor((height - offset_y) / TILESIZE) + 1;// offset needs added
	
	g_debug("Map width %i", width);

	// tile x0, x0-xn, y0-yn
	tile_x0 =  floor((float)pixel_x / (float)TILESIZE);
	tile_y0 =  floor((float)pixel_y / (float)TILESIZE);
	//if(pixel_x < 0) tile_x0
	
	//TODO: implement wrap around
	

	for (i=tile_x0; i<(tile_x0+tiles_nx);i++)
	{
		for (j=tile_y0;  j<(tile_y0+tiles_ny); j++)
		{
			g_debug("+++++++x,y: %d,%d -- %d, %d -- %d, %d",i,j,pixel_x,pixel_y,offset_x,offset_y);

			if(	j<0	|| i<0 || i>=exp(zoom * M_LN2) || j>=exp(zoom * M_LN2))
			{
				gdk_draw_rectangle (priv->pixmap,
									GTK_WIDGET(map)->style->white_gc,
									TRUE,
									offset_xn, offset_yn,
									TILESIZE,TILESIZE);
				
				gtk_widget_queue_draw_area (GTK_WIDGET(map), 
											offset_xn,offset_yn,
											TILESIZE,TILESIZE);
			}
			else
			{
				osm_gps_map_load_tile(map,
									  zoom,
									  i,j,
									  offset_xn,offset_yn);
			}
			offset_yn += TILESIZE;
		}
		offset_xn += TILESIZE;
		offset_yn = offset_y;
	}
}

static void
osm_gps_map_fill_tiles_latlon (OsmGpsMap *map, float lat, float lon, int zoom)
{
	int pixel_x, pixel_y;
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);
	
	// pixel_x,y, offsets
	pixel_x = lon2pixel(zoom, lon);
	pixel_y = lat2pixel(zoom, lat);
	g_debug("fill_tiles_latlon(): lat %f  %i -- lon %f  %i",lat,pixel_y,lon,pixel_x);
	
	osm_gps_map_fill_tiles_pixel (map, pixel_x, pixel_y, zoom);
}

G_DEFINE_TYPE (OsmGpsMap, osm_gps_map, GTK_TYPE_DRAWING_AREA);

static void
osm_gps_map_init (OsmGpsMap *object)
{
	/* TODO: Add initialization code here */
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(object);
	
	priv->invert_zoom = FALSE;
	
	priv->pixmap = NULL;
	priv->gc_map = NULL;
	
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
	
	//TODO: Change naumber of concurrent connections option
	priv->soup_session = soup_session_async_new_with_options(SOUP_SESSION_USER_AGENT,
															 "Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.8.1.11) Gecko/20071127 Firefox/2.0.0.11",
															 NULL);
	
	gtk_widget_add_events (GTK_WIDGET (object),
			GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK);
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
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(object);
	
	switch (prop_id)
	{
	case PROP_AUTO_CENTER:
		priv->global_autocenter = g_value_get_boolean (value);
		break;
	case PROP_AUTO_DOWNLOAD:
		priv->global_auto_download = g_value_get_boolean (value);
		break;
	case PROP_REPO_URI:
		priv->repo_uri = g_value_dup_string (value);
		break;
	case PROP_TILE_CACHE:
		priv->cache_dir = g_value_dup_string (value);
		break;
	case PROP_ZOOM:
		priv->global_zoom = g_value_get_int (value);
		break;
	case PROP_INVERT_ZOOM:
		priv->invert_zoom = g_value_get_boolean (value);
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
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
	GtkDrawingAreaClass* parent_class = GTK_DRAWING_AREA_CLASS (klass);

	g_type_class_add_private (klass, sizeof (OsmGpsMapPrivate));

	object_class->finalize = osm_gps_map_finalize;
	object_class->set_property = osm_gps_map_set_property;
	object_class->get_property = osm_gps_map_get_property;
	
	widget_class->expose_event = osm_gps_map_expose;
	widget_class->configure_event = osm_gps_map_configure;
	widget_class->button_press_event = osm_gps_map_button_press;
	widget_class->button_release_event = osm_gps_map_button_release;
	widget_class->motion_notify_event = osm_gps_map_motion_notify;
	widget_class->scroll_event = osm_gps_map_scroll;

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
	                                                      "http://tile.openstreetmap.org/%d/%d/%d.png",
	                                                      G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));

	g_object_class_install_property (object_class,
	                                 PROP_TILE_CACHE,
	                                 g_param_spec_string ("tile-cache",
	                                                      "tile cache",
	                                                      "osm local tile cache dir",
	                                                      "/tmp/Maps/OSM",
	                                                      G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));

	g_object_class_install_property (object_class,
	                                 PROP_ZOOM,
	                                 g_param_spec_int ("zoom",
	                                                    "zoom",
	                                                    "zoom level",
	                                                    0, /* minimum property value */
	                                                    17, /* maximum property value */
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
	bbox_pixel_t bbox_pixel;
	int zoom;
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);

	zoom_end = (zoom_end > 17) ? 17 : zoom_end;
	g_debug("Download maps: z:%d->%d",zoom_start, zoom_end);
	
	for(zoom=zoom_start; zoom<=zoom_end; zoom++)
	{
		bbox_pixel = osm_gps_map_get_bbox_pixel(map, bbox, zoom);
		osm_gps_map_queue_tile_dl_for_bbox(map, bbox_pixel,zoom);
	}
}

void
osm_gps_map_download_tile (OsmGpsMap *map, int zoom, int x, int y, int offset_x, int offset_y)
{
	SoupMessage *msg;
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);
	tile_download_t *dl = g_new0(tile_download_t,1);
	
	if (!priv->invert_zoom)
		dl->uri = g_strdup_printf(priv->repo_uri, zoom, x, y);
	else
		dl->uri = g_strdup_printf(priv->repo_uri, x, y, 17-zoom);	

	dl->folder = g_strdup_printf("%s/%d/%d/",priv->cache_dir, zoom, x);
	dl->filename = g_strdup_printf("%s/%d/%d/%d.png",priv->cache_dir, zoom, x, y);
	dl->map = map;
	dl->offset_x = offset_x;
	dl->offset_y = offset_y;

	g_debug("Download tile: %d,%d z:%d\n\t%s --> %s", x, y, zoom, dl->uri, dl->filename);
	
	msg = soup_message_new (SOUP_METHOD_GET, dl->uri);
	if (msg) {
		soup_session_queue_message (priv->soup_session, msg, osm_gps_map_tile_download_complete, dl);
	} else {
		g_warning("Could not create soup message");
		g_free(dl->uri);
		g_free(dl->folder);
		g_free(dl->filename);
		g_free(dl);
	}
}

bbox_t
osm_gps_map_get_bbox (OsmGpsMap *map)
{
	bbox_t bbox;
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);
	
	g_debug("Get bbox");

	bbox.lat1 = pixel2lat(priv->global_zoom, priv->global_y);
	bbox.lon1 = pixel2lon(priv->global_zoom, priv->global_x);
	bbox.lat2 = pixel2lat(priv->global_zoom, priv->global_y + priv->global_drawingarea_height);
	bbox.lon2 = pixel2lon(priv->global_zoom, priv->global_x + priv->global_drawingarea_width);

	g_debug("BBOX: %f %f %f %f", bbox.lat1, bbox.lon1, bbox.lat2, bbox.lon2);
	
	return bbox;
}

void
osm_gps_map_map_redraw (OsmGpsMap *map)
{
	//TODO: There is no real need to run this on a timer. We should be smart enough
	//to only redraw from the correct functions (like map resized, gps arrive, etc)
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);

	g_debug("REPAINTING.............");
	osm_gps_map_fill_tiles_pixel(map, priv->global_x, priv->global_y, priv->global_zoom);

	//print_track();
	//paint_friends();
	//paint_photos();
	//paint_pois();
	//osd_speed();
}

void
osm_gps_map_set_mapcenter (OsmGpsMap *map, float lat, float lon, int zoom)
{
	int pixel_x, pixel_y;
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);
	
	lat = deg2rad(lat);
	lon = deg2rad(lon);
	
	// pixel_x,y, offsets
	pixel_x = lon2pixel(zoom, lon);
	pixel_y = lat2pixel(zoom, lat);

	g_debug("fill_tiles_latlon(): lat %f  %i -- lon %f  %i",lat,pixel_y,lon,pixel_x);
	
	//osd_speed();
	osm_gps_map_fill_tiles_pixel (map,
								  pixel_x - priv->global_drawingarea_width/2,
								  pixel_y - priv->global_drawingarea_height/2,
								  zoom);
	//print_track();
	//paint_friends();
	//paint_photos();
	//paint_pois();
	//osd_speed(); //TODO add missing queue or soemthing...
}

void osm_gps_map_set_zoom (OsmGpsMap *map, int zoom)
{
	int zoom_old;
	double factor;
	int width_center, height_center;
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);
	
	if(priv->global_zoom<17)
	{	
		width_center  = GTK_WIDGET(map)->allocation.width / 2;
		height_center = GTK_WIDGET(map)->allocation.height / 2;
		
		zoom_old = priv->global_zoom++;
		factor = exp(priv->global_zoom * M_LN2)/exp(zoom_old * M_LN2);
		
		g_debug("zoom changed from %d to %d factor:%f x:%d", zoom_old,priv->global_zoom, factor, priv->global_x);
	
		// hier
		//global_x *= factor;
		//global_y *= factor;
		
		priv->global_x = ((priv->global_x + width_center) * factor) - width_center;
		priv->global_y = ((priv->global_y + height_center) * factor) - height_center;
		
		osm_gps_map_fill_tiles_pixel(map, priv->global_x, priv->global_y, priv->global_zoom);
	
		//FIXME TODO: wenn zoom ändert, müssen sich auch globalx y ändern
		//print_track();
		//paint_friends();
		//paint_photos();
		//paint_pois();
	}
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
	tile_t tile;
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);
	
	tile.x =  (int)floor((float)pixel_x / (float)TILESIZE);
	tile.y =  (int)floor((float)pixel_y / (float)TILESIZE);
	tile.zoom = zoom;
	//tile.repo = global_curr_repo->data;
	
	return tile;
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

GtkWidget *
osm_gps_map_new (void)
{
	return g_object_new (OSM_TYPE_GPS_MAP, NULL);
}
