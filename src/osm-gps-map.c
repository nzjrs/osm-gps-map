/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * osm-gps-map.c
 * Copyright (C) Marcus Bauer 2008 <marcus.bauer@gmail.com>
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

#include <fcntl.h>
#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gdk/gdk.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <libsoup/soup.h>

#include "converter.h"
#include "osm-gps-map-types.h"
#include "osm-gps-map.h"

typedef struct _OsmGpsMapPrivate OsmGpsMapPrivate;
struct _OsmGpsMapPrivate
{
	SoupSession * soup_session;

	GHashTable * tile_queue;
	GHashTable * missing_tiles;

	char * cache_dir;
	char * repo_uri;
	gboolean invert_zoom;
	int global_zoom;
	int max_zoom;
	gboolean global_autocenter;
	gboolean global_auto_download;
	int global_x;
	int global_y;

	gboolean trip_counter;
	GSList *trip;
	GSList *images;

	//Used for storing the joined tiles
	GdkPixmap *pixmap;
	GdkGC *gc_map;
	
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

typedef struct {
	int x;
	int y;
	int zoom;
} tile_t;

typedef struct {
	coord_t pt;
	GdkPixbuf *image;
	int w;
	int h;
} image_t;

enum
{
	PROP_0,

	PROP_AUTO_CENTER,
	PROP_AUTO_DOWNLOAD,
	PROP_REPO_URI,
	PROP_PROXY_URI,
	PROP_TILE_CACHE,
	PROP_ZOOM,
	PROP_MAX_ZOOM,
	PROP_INVERT_ZOOM,
	PROP_LATITUDE,
	PROP_LONGITUDE,
	PROP_MAP_X,
	PROP_MAP_Y,
	PROP_TILES_QUEUED
};

static void osm_gps_map_fill_tiles_pixel (OsmGpsMap *map, int pixel_x, int pixel_y, int zoom);
static tile_t osm_gps_map_get_tile (OsmGpsMap *map, int pixel_x, int pixel_y, int zoom);
void osm_gps_map_download_tile (OsmGpsMap *map, int zoom, int x, int y, int offset_x, int offset_y);
void osm_gps_map_print_images (OsmGpsMap *map);
void osm_gps_map_map_redraw (OsmGpsMap *map);

/*
 * Description:
 *   Find and replace text within a string.
 *
 * Parameters:
 *   src  (in) - pointer to source string
 *   from (in) - pointer to search text
 *   to   (in) - pointer to replacement text
 *
 * Returns:
 *   Returns a pointer to dynamically-allocated memory containing string
 *   with occurences of the text pointed to by 'from' replaced by with the
 *   text pointed to by 'to'.
 */
static gchar *
replace_string(const gchar *src, const gchar *from, const gchar *to)
{
	size_t size    = strlen(src) + 1;
	size_t fromlen = strlen(from);
	size_t tolen   = strlen(to);

	/* Allocate the first chunk with enough for the original string. */
	gchar *value = g_malloc(size);


    /* We need to return 'value', so let's make a copy to mess around with. */
	gchar *dst = value;

	if ( value != NULL )
	{
		for ( ;; )
		{
			/* Try to find the search text. */
			const gchar *match = g_strstr_len(src, size, from);
			if ( match != NULL )
			{
				gchar *temp;
				 /* Find out how many characters to copy up to the 'match'. */
				size_t count = match - src;


				/* Calculate the total size the string will be after the
				 * replacement is performed. */
				size += tolen - fromlen;

				temp = g_realloc(value, size);
				if ( temp == NULL )
				{
				   g_free(value);
				   return NULL;
				}

				/* we'll want to return 'value' eventually, so let's point it 
				 * to the memory that we are now working with. 
				 * And let's not forget to point to the right location in 
				 * the destination as well. */
				dst = temp + (dst - value);
				value = temp;

				/*
				 * Copy from the source to the point where we matched. Then
				 * move the source pointer ahead by the amount we copied. And
				 * move the destination pointer ahead by the same amount.
				 */
				g_memmove(dst, src, count);
				src += count;
				dst += count;

				 /* Now copy in the replacement text 'to' at the position of
				 * the match. Adjust the source pointer by the text we replaced.
				 * Adjust the destination pointer by the amount of replacement
				 * text. */
				g_memmove(dst, to, tolen);
				src += fromlen;
				dst += tolen;
			}
			else
			{
		        /*
		         * Copy any remaining part of the string. This includes the null
		         * termination character.
		         */
		        strcpy(dst, src);
		        break;
			}
		}
	}
	return value;
}

static gchar *
replace_map_uri(const gchar *uri, int zoom, int x, int y)
{
	gchar *zs,*xs,*ys,*fz,*fx,*fy;

	zs = g_strdup_printf("%d", zoom);
	xs = g_strdup_printf("%d", x);
	ys = g_strdup_printf("%d", y);

	fz = replace_string(uri, "#Z", zs);
	fx = replace_string(fz, "#X", xs);
	fy = replace_string(fx, "#Y", ys);

	g_free(zs);   
	g_free(xs);
	g_free(ys);

	g_free(fz);
	g_free(fx);

	return fy;
}

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
	
		osm_gps_map_map_redraw(OSM_GPS_MAP(widget));
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

	// are we being dragged
	if (state & GDK_BUTTON1_MASK) {
		// yes, and we have dragged more than 6 pixels 
		if (priv->wtfcounter >= 6) {
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

			//Paint white to the top and left of the map if dragging. Its less
			//ugly than painting the corrupted map		
			if(priv->mouse_dx>0) {
				gdk_draw_rectangle (
					widget->window,
					widget->style->white_gc,
					TRUE,
					0, 0,
					priv->mouse_dx,
					widget->allocation.height);
			}
		
			if (priv->mouse_dy>0) {
				gdk_draw_rectangle (
					widget->window,
					widget->style->white_gc,
					TRUE,
					0, 0,
					widget->allocation.width,
					priv->mouse_dy);
			}

			g_debug("motion: %i %i - start: %i %i - dx: %i %i --wtf %i\n", x,y, priv->mouse_x, priv->mouse_y, priv->mouse_dx, priv->mouse_dy, priv->wtfcounter);
		} else {
			priv->wtfcounter++;
		}
	}

	return FALSE;
}

gboolean
osm_gps_map_configure (GtkWidget *widget, GdkEventConfigure *event)
{
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(widget);

	g_debug("CONFIGURE");	

	/* create pixmap */
	if (priv->pixmap)
		g_object_unref (priv->pixmap);

	priv->pixmap = gdk_pixmap_new (
			widget->window,
			widget->allocation.width+260, //TODO: this could be cleverer
			widget->allocation.height+260,
			-1);

	/* and gc, used for clipping (I think......) */
	if(priv->gc_map)
		g_object_unref(priv->gc_map);

	priv->gc_map = gdk_gc_new(priv->pixmap);

	osm_gps_map_map_redraw(OSM_GPS_MAP(widget));

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
	
	/* draw pixbuf onto pixmap */
	gdk_draw_pixbuf (priv->pixmap,
					 priv->gc_map,
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
	OsmGpsMap *map = OSM_GPS_MAP(dl->map);
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);
	
	if (SOUP_STATUS_IS_SUCCESSFUL (msg->status_code)) 
	{
		if (g_mkdir_with_parents(dl->folder,0700) == 0) 
		{
			fd = open(dl->filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
			if (fd != -1) {
				write (fd, msg->response_body->data, msg->response_body->length);
				g_debug("Wrote %lld bytes to %s", msg->response_body->length, dl->filename);
				close (fd);
		
				/* Redraw the area of the screen */
				if ( msg->response_body->length > 0 ) {
					GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file (dl->filename, NULL);
					if(pixbuf) {
						g_debug("Found tile %s", dl->filename);
						osm_gps_map_blit_tile(map, pixbuf, dl->offset_x,dl->offset_y);
						g_object_unref (pixbuf);
					}
				}
			}
		}
		else 
		{
			g_warning("Error creating tile download directory: %s", dl->folder);
		}

	g_hash_table_remove(priv->tile_queue, dl->uri);
	g_free(dl->uri);
	g_free(dl->folder);
	g_free(dl->filename);
	g_free(dl);


	} 
	else 
	{
		g_warning("Error downloading tile: %d - %s", msg->status_code, msg->reason_phrase);
		if (msg->status_code == SOUP_STATUS_NOT_FOUND) 
		{
			g_hash_table_insert(priv->missing_tiles, dl->uri, NULL);
			g_hash_table_remove(priv->tile_queue, dl->uri);
		}
		else if (msg->status_code == SOUP_STATUS_CANCELLED)
		{
			//application exiting
			;
		}
		else 
		{
			soup_session_requeue_message(session, msg);
			return;
		}
	}


}

static void
osm_gps_map_queue_tile_dl_for_bbox (OsmGpsMap *map, bbox_pixel_t bbox_pixel, int zoom)
{
	tile_t tile_11, tile_22;
	int i,j,num_tiles=0;
	
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
			//osm_gps_map_download_tile(map, zoom, i, j, -1, -1);
			num_tiles++;
		}
	}
	g_debug("DL %d",num_tiles);
}

static bbox_pixel_t
osm_gps_map_get_bbox_pixel (OsmGpsMap *map, coord_t *pt1, coord_t *pt2, int zoom)
{
	bbox_pixel_t bbox_pixel;
	
	bbox_pixel.x1 = lon2pixel(zoom, pt1->lon);
	bbox_pixel.y1 = lat2pixel(zoom, pt1->lat);
	bbox_pixel.x2 = lon2pixel(zoom, pt2->lon);
	bbox_pixel.y2 = lat2pixel(zoom, pt2->lat);

	g_debug("1:%d,%d 2:%d,%d",bbox_pixel.x1,bbox_pixel.y1,bbox_pixel.x2,bbox_pixel.y2);
	
	return	bbox_pixel;
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

/*
void
osm_gps_map_fill_tiles_latlon (OsmGpsMap *map, float lat, float lon, int zoom)
{
	int pixel_x, pixel_y;
	
	// pixel_x,y, offsets
	pixel_x = lon2pixel(zoom, lon);
	pixel_y = lat2pixel(zoom, lat);
	g_debug("fill_tiles_latlon(): lat %f  %i -- lon %f  %i",lat,pixel_y,lon,pixel_x);
	
	osm_gps_map_fill_tiles_pixel (map, pixel_x, pixel_y, zoom);
}*/

G_DEFINE_TYPE (OsmGpsMap, osm_gps_map, GTK_TYPE_DRAWING_AREA);

static void
osm_gps_map_init (OsmGpsMap *object)
{
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(object);

	priv->pixmap = NULL;

	priv->trip_counter = TRUE;
	priv->trip = NULL;
	priv->images = NULL;
	
	priv->wtfcounter = 0;
	priv->mouse_dx = 0;
	priv->mouse_dy = 0;
	priv->mouse_x = 0;
	priv->mouse_y = 0;

	//TODO: Change naumber of concurrent connections option
	priv->soup_session = soup_session_async_new_with_options(SOUP_SESSION_USER_AGENT,
															 "Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.8.1.11) Gecko/20071127 Firefox/2.0.0.11",
															 NULL);


	//Hash table which maps tile d/l URIs to SoupMessage requests
	priv->tile_queue = g_hash_table_new (g_str_hash, g_str_equal);

	//Some mapping providers (Google) have varying degrees of tiles at multiple
	//zoom levels
	priv->missing_tiles = g_hash_table_new (g_str_hash, g_str_equal);

	
	gtk_widget_add_events (GTK_WIDGET (object),
			GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK);
}

static void
osm_gps_map_finalize (GObject *object)
{
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(object);

	soup_session_abort(priv->soup_session);
	g_object_unref(priv->soup_session);

	g_hash_table_destroy(priv->tile_queue);
	g_hash_table_destroy(priv->missing_tiles);

	g_free(priv->cache_dir);
	g_free(priv->repo_uri);

	//clears the trip list and all resources
	if (priv->trip) {
		g_slist_foreach(priv->trip, (GFunc) g_free, NULL);
		g_slist_free(priv->trip);
	}

	//free the poi image lists
	if (priv->images) {
		GSList *list;
		for(list = priv->images; list != NULL; list = list->next)
		{
			image_t *im = list->data;
			g_object_unref(im->image);
			g_free(im);
		}
		g_slist_free(priv->images);
	}

	if(priv->pixmap)
		g_object_unref (priv->pixmap);

	if(priv->gc_map)
		g_object_unref(priv->gc_map);

	g_debug("POOF");
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
	case PROP_PROXY_URI:
		if ( g_value_get_string(value) ) {
			GValue val = {0};
			const char *proxy_uri;

			proxy_uri = g_value_get_string(value);
			g_debug("Setting proxy server: %s", proxy_uri);

			SoupURI* uri = soup_uri_new(proxy_uri);
			g_value_init(&val, SOUP_TYPE_URI);
			g_value_take_boxed(&val, uri);

			g_object_set_property(G_OBJECT(priv->soup_session),SOUP_SESSION_PROXY_URI,&val);
		}

		break;
	case PROP_TILE_CACHE:
		priv->cache_dir = g_value_dup_string (value);
		break;
	case PROP_ZOOM:
		priv->global_zoom = g_value_get_int (value);
		break;
	case PROP_MAX_ZOOM:
		priv->max_zoom = g_value_get_int (value);
		break;
	case PROP_INVERT_ZOOM:
		priv->invert_zoom = g_value_get_boolean (value);
		break;
	case PROP_MAP_X:
		priv->global_x = g_value_get_int (value);
		break;
	case PROP_MAP_Y:
		priv->global_y = g_value_get_int (value);
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
	float lat,lon;
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(object);
	OsmGpsMap *map = OSM_GPS_MAP(object);

	switch (prop_id)
	{
	case PROP_AUTO_CENTER:
		g_value_set_boolean(value, priv->global_autocenter);
		break;
	case PROP_AUTO_DOWNLOAD:
		g_value_set_boolean(value, priv->global_auto_download);
		break;
	case PROP_REPO_URI:
		g_value_set_string(value, priv->repo_uri);
		break;
	case PROP_TILE_CACHE:
		g_value_set_string(value, priv->cache_dir);
		break;
	case PROP_ZOOM:
		g_value_set_int(value, priv->global_zoom);
		break;
	case PROP_MAX_ZOOM:
		g_value_set_int(value, priv->max_zoom);
		break;
	case PROP_INVERT_ZOOM:
		g_value_set_boolean(value, priv->invert_zoom);
		break;
	case PROP_LATITUDE:
		lat = pixel2lat(priv->global_zoom,
						priv->global_y + (GTK_WIDGET(map)->allocation.height / 2));
		g_value_set_float(value, rad2deg(lat));
		break;
	case PROP_LONGITUDE:
		lon = pixel2lon(priv->global_zoom,
						priv->global_x + (GTK_WIDGET(map)->allocation.width / 2));
		g_value_set_float(value, rad2deg(lon));
		break;
	case PROP_MAP_X:
		g_value_set_int(value, priv->global_x);
		break;
	case PROP_MAP_Y:
		g_value_set_int(value, priv->global_y);
		break;
	case PROP_TILES_QUEUED:
		g_value_set_int(value, g_hash_table_size(priv->tile_queue));
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
	                                                      "http://tile.openstreetmap.org/#Z/#X/#Y.png",
	                                                      G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));

	g_object_class_install_property (object_class,
	                                 PROP_PROXY_URI,
	                                 g_param_spec_string ("proxy-uri",
	                                                      "proxy uri",
	                                                      "http proxy uri on NULL",
	                                                      NULL,
	                                                      G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

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
	                                                    DEFAULT_MAX_ZOOM, /* maximum property value */
	                                                    3,
	                                                    G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property (object_class,
	                                 PROP_MAX_ZOOM,
	                                 g_param_spec_int ("max-zoom",
	                                                    "max zoom",
	                                                    "maximum zoom level",
	                                                    0, /* minimum property value */
	                                                    DEFAULT_MAX_ZOOM, /* maximum property value */
	                                                    DEFAULT_MAX_ZOOM,
	                                                    G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property (object_class,
	                                 PROP_INVERT_ZOOM,
	                                 g_param_spec_boolean ("invert-zoom",
	                                                       "invert zoom",
	                                                       "is zoom inverted",
	                                                       FALSE,
	                                                       G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));

	g_object_class_install_property (object_class,
	                                 PROP_LATITUDE,
	                                 g_param_spec_float ("latitude",
	                                                    "latitude",
	                                                    "latitude in degrees",
	                                                    -90.0, /* minimum property value */
	                                                    90.0, /* maximum property value */
	                                                    0,
	                                                    G_PARAM_READABLE));

	g_object_class_install_property (object_class,
	                                 PROP_LONGITUDE,
	                                 g_param_spec_float ("longitude",
	                                                    "longitude",
	                                                    "longitude in degrees",
	                                                    -180.0, /* minimum property value */
	                                                    180.0, /* maximum property value */
	                                                    0,
	                                                    G_PARAM_READABLE));

	g_object_class_install_property (object_class,
	                                 PROP_MAP_X,
	                                 g_param_spec_int ("map-x",
	                                                    "map-x",
	                                                    "initial map x location",
	                                                    G_MININT, /* minimum property value */
	                                                    G_MAXINT, /* maximum property value */
	                                                    890,
	                                                    G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property (object_class,
	                                 PROP_MAP_Y,
	                                 g_param_spec_int ("map-y",
	                                                    "map-y",
	                                                    "initial map y location",
	                                                    G_MININT, /* minimum property value */
	                                                    G_MAXINT, /* maximum property value */
	                                                    515,
	                                                    G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property (object_class,
	                                 PROP_TILES_QUEUED,
	                                 g_param_spec_int ("tiles-queued",
	                                                    "tiles-queued",
	                                                    "number of tiles currently waiting to download",
	                                                    G_MININT, /* minimum property value */
	                                                    G_MAXINT, /* maximum property value */
	                                                    0,
	                                                    G_PARAM_READABLE));

}


void
osm_gps_map_download_maps (OsmGpsMap *map, coord_t *pt1, coord_t *pt2, int zoom_start, int zoom_end)
{
	bbox_pixel_t bbox_pixel;
	int zoom;
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);
	
	if (pt1 && pt2) {

		zoom_end = CLAMP(zoom_end, 1, priv->max_zoom);
		g_debug("Download maps: z:%d->%d",zoom_start, zoom_end);
	
		for(zoom=zoom_start; zoom<=zoom_end; zoom++)
		{
			bbox_pixel = osm_gps_map_get_bbox_pixel(map, pt1, pt2, zoom);
			osm_gps_map_queue_tile_dl_for_bbox(map, bbox_pixel,zoom);
		}
	}
}

void
osm_gps_map_download_tile (OsmGpsMap *map, int zoom, int x, int y, int offset_x, int offset_y)
{
	SoupMessage *msg;
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);
	tile_download_t *dl = g_new0(tile_download_t,1);
	
	if (!priv->invert_zoom)
		dl->uri = replace_map_uri(priv->repo_uri, zoom, x, y);
	else
		dl->uri = replace_map_uri(priv->repo_uri, priv->max_zoom-zoom, x, y);
		
	//check the tile has not already been queued for download, 
	//or has been attempted, and its missing
	if ( 	g_hash_table_lookup_extended(priv->tile_queue, dl->uri, NULL, NULL) ||
			g_hash_table_lookup_extended(priv->missing_tiles, dl->uri, NULL, NULL) ) 
	{
		g_debug("Tile already downloading (or missing)");
	    g_free(dl->uri);
		g_free(dl);
	} else {
		dl->folder = g_strdup_printf("%s/%d/%d/",priv->cache_dir, zoom, x);
		dl->filename = g_strdup_printf("%s/%d/%d/%d.png",priv->cache_dir, zoom, x, y);
		dl->map = map;
		dl->offset_x = offset_x;
		dl->offset_y = offset_y;

		g_debug("Download tile: %d,%d z:%d\n\t%s --> %s", x, y, zoom, dl->uri, dl->filename);
	
		msg = soup_message_new (SOUP_METHOD_GET, dl->uri);
		if (msg) {
			g_hash_table_insert (priv->tile_queue, dl->uri, msg);
			soup_session_queue_message (priv->soup_session, msg, osm_gps_map_tile_download_complete, dl);
		} else {
			g_warning("Could not create soup message");
			g_free(dl->uri);
			g_free(dl->folder);
			g_free(dl->filename);
			g_free(dl);
		}
	}
}

void
osm_gps_map_get_bbox (OsmGpsMap *map, coord_t *pt1, coord_t *pt2)
{
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);
	
	if (pt1 && pt2) {
		pt1->lat = pixel2lat(priv->global_zoom, priv->global_y);
		pt1->lon = pixel2lon(priv->global_zoom, priv->global_x);
		pt2->lat = pixel2lat(priv->global_zoom, priv->global_y + GTK_WIDGET(map)->allocation.height);
		pt2->lon = pixel2lon(priv->global_zoom, priv->global_x + GTK_WIDGET(map)->allocation.width);

		g_debug("BBOX: %f %f %f %f", pt1->lat, pt1->lon, pt2->lat, pt2->lon);
	}
}

void
osm_gps_map_map_redraw (OsmGpsMap *map)
{
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);
	g_debug("REPAINTING.............");

	/* draw white background to initialise pixmap */
	gdk_draw_rectangle (
		priv->pixmap,
		GTK_WIDGET(map)->style->white_gc,
		TRUE,
		0, 0,
		GTK_WIDGET(map)->allocation.width+260,
		GTK_WIDGET(map)->allocation.height+260);

/*				
	gtk_widget_queue_draw_area (
		GTK_WIDGET(map), 
		0,0,
		GTK_WIDGET(map)->allocation.width+260,
		GTK_WIDGET(map)->allocation.height+260);
*/

	osm_gps_map_fill_tiles_pixel(
		map,
		priv->global_x,
		priv->global_y,
		priv->global_zoom);

	if (priv->trip_counter)
		osm_gps_map_print_track (map, priv->trip);
		
	osm_gps_map_print_images(map);
}

void
osm_gps_map_set_mapcenter (OsmGpsMap *map, float lat, float lon, int zoom)
{
	int pixel_x, pixel_y;
	float rlat, rlon;
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);
	
	rlat = deg2rad(lat);
	rlon = deg2rad(lon);
	
	// pixel_x,y, offsets
	pixel_x = lon2pixel(zoom, rlon);
	pixel_y = lat2pixel(zoom, rlat);

	priv->global_x = pixel_x - GTK_WIDGET(map)->allocation.width/2;
	priv->global_y = pixel_y - GTK_WIDGET(map)->allocation.height/2;

	//constrain zoom 1 -> max_zoom
	priv->global_zoom = CLAMP(zoom, 1, priv->max_zoom);

	osm_gps_map_map_redraw(map);
}

int osm_gps_map_set_zoom (OsmGpsMap *map, int zoom)
{
	int zoom_old;
	double factor;
	int width_center, height_center;
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);

	if (zoom != priv->global_zoom)
	{	
		width_center  = GTK_WIDGET(map)->allocation.width / 2;
		height_center = GTK_WIDGET(map)->allocation.height / 2;
		
		zoom_old = priv->global_zoom;
		//constrain zoom 1 -> max_zoom
		priv->global_zoom = CLAMP(zoom, 1, priv->max_zoom); 
		factor = exp(priv->global_zoom * M_LN2)/exp(zoom_old * M_LN2);
		
		g_debug("zoom changed from %d to %d factor:%f x:%d", zoom_old,priv->global_zoom, factor, priv->global_x);
	
		// hier
		//global_x *= factor;
		//global_y *= factor;
		
		priv->global_x = ((priv->global_x + width_center) * factor) - width_center;
		priv->global_y = ((priv->global_y + height_center) * factor) - height_center;
		
		osm_gps_map_map_redraw(map);
	}
	return priv->global_zoom;
}

void
osm_gps_map_print_track (OsmGpsMap *map, GSList *trackpoint_list)
{
	GSList *list;
	int pixel_x, pixel_y, x,y, last_x = 0, last_y = 0;
	int min_x = 0,min_y = 0,max_x = 0,max_y = 0;
	float lat, lon;
	GdkColor color;
	GdkGC *gc;
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);
	
	// A red line
	gc = gdk_gc_new(priv->pixmap);
	color.green = 0;
	color.blue = 0;
	color.red = 60000;
	gdk_gc_set_rgb_fg_color(gc, &color);
	gdk_gc_set_line_attributes(gc,
		5, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);

	for(list = trackpoint_list; list != NULL; list = list->next)
	{
		coord_t *tp = list->data;
	
		lat = tp->lat;
		lon = tp->lon;
		
		// pixel_x,y, offsets
		pixel_x = lon2pixel(priv->global_zoom, lon);
		pixel_y = lat2pixel(priv->global_zoom, lat);
		
		x = pixel_x - priv->global_x;
		y = pixel_y - priv->global_y;
		
		// first time through loop
		if (list == trackpoint_list) {
			last_x = x;
			last_y = y;
		}

/*		gdk_draw_arc (
			pixmap,
			//map_drawable->style->black_gc,
			gc,
			TRUE,			//filled
			x-4, y-4,		// x,y
			8,8,			// width, height
			0,23040);		// start-end angle 64th, from 3h, anti clockwise
*/

		gdk_draw_line (priv->pixmap, gc, x, y, last_x, last_y);
		last_x = x;
		last_y = y;
		
		max_x = MAX(x,max_x);
		min_x = MIN(x,min_x);
		max_y = MAX(y,max_y);
		min_y = MIN(y,min_y);
		//printf("Trackpoint: lat %f - lon %f\n",tp->lat, tp->lon);
	}
	
	// TODO last segment to current pos.
	// globalx?
	//gdk_draw_line (pixmap, gc, x, y, last_x, last_y);
	/*
	NOT READY FOR PRIME TIME

	if(	
		gpsdata && 
		gpsdata->fix.longitude !=0 &&  
		gpsdata->fix.latitude != 0 &&
		last_x != 0 &&
		last_y != 0
		)
	{
		pixel_x = lon2pixel(global_zoom, deg2rad(gpsdata->fix.longitude));
		pixel_y = lat2pixel(global_zoom, deg2rad(gpsdata->fix.latitude));
			
		x = pixel_x - global_x;
		y = pixel_y - global_y;

	
		gdk_draw_line (pixmap, gc, x, y, last_x, last_y);
printf("LINE x y lx ly: %d %d %d %d\n",x,y,last_x,last_y);	
			gtk_widget_queue_draw_area (
			map_drawable, 
			x-4, y-4,
			8,8);
	} */
	
		// Also account for line width (5)
		gtk_widget_queue_draw_area (
			GTK_WIDGET(map), 
			min_x-5, min_y-5,
			max_x+10, max_y+10);

}

void
osm_gps_map_print_images (OsmGpsMap *map)
{
	GSList *list;
	int x,y,pixel_x,pixel_y;
	int min_x = 0,min_y = 0,max_x = 0,max_y = 0;
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);
	
	for(list = priv->images; list != NULL; list = list->next)
	{
		image_t *im = list->data;

		// pixel_x,y, offsets
		pixel_x = lon2pixel(priv->global_zoom, deg2rad(im->pt.lon));
		pixel_y = lat2pixel(priv->global_zoom, deg2rad(im->pt.lat));
		
		g_debug("image %dx%d @: %f,%f (%d,%d)",
					im->w, im->h,
					im->pt.lat, im->pt.lon,
					pixel_x, pixel_y);
		
		x = pixel_x - priv->global_x;
		y = pixel_y - priv->global_y;
		
		gdk_draw_pixbuf (
					priv->pixmap,
					priv->gc_map,
					im->image,
					0,0,
					x-(im->w/2),y-(im->h/2),
					im->w,im->h,
					GDK_RGB_DITHER_NONE, 0, 0);
			
		max_x = MAX(x+im->w,max_x);
		min_x = MIN(x-im->w,min_x);
		max_y = MAX(y+im->h,max_y);
		min_y = MIN(y-im->h,min_y);
	}

	gtk_widget_queue_draw_area (
			GTK_WIDGET(map), 
			min_x, min_y,
			max_x, max_y);


}

void
osm_gps_map_add_image (OsmGpsMap *map, float lat, float lon, GdkPixbuf *image)
{
	image_t *im;
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);
	
	if (image) {
		//cache w/h for speed, and add image to list
		im = g_new0(image_t,1);
		im->w = gdk_pixbuf_get_width(image);
		im->h = gdk_pixbuf_get_width(image);
		im->pt.lat = lat;
		im->pt.lon = lon;

		g_object_ref(image);
		im->image = image;
	
		priv->images = g_slist_append(priv->images, im);
		
		osm_gps_map_print_images(map);
	}
}

tile_t
osm_gps_map_get_tile (OsmGpsMap *map, int pixel_x, int pixel_y, int zoom)
{
	tile_t tile;
	
	tile.x =  (int)floor((float)pixel_x / (float)TILESIZE);
	tile.y =  (int)floor((float)pixel_y / (float)TILESIZE);
	tile.zoom = zoom;
	
	return tile;
}

void
osm_gps_map_osd_speed (OsmGpsMap *map, float speed)
{
	/* TODO: Add public function implementation here */
}

void
osm_gps_map_draw_gps (OsmGpsMap *map, float latitude, float longitude, float heading)
{
	//In radians
	float rlat, rlon;
	int pixel_x, pixel_y, x, y;
	GdkColor color;
	GdkGC *marker;
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);

	rlat = deg2rad(latitude);
	rlon = deg2rad(longitude);

	// pixel_x,y, offsets
	pixel_x = lon2pixel(priv->global_zoom, rlon);
	pixel_y = lat2pixel(priv->global_zoom, rlat);

	x = pixel_x - priv->global_x;
	y = pixel_y - priv->global_y;

	//If trip marker add to list of gps points.
	if (priv->trip_counter) {
		coord_t *tp = g_new0(coord_t,1);
		tp->lat = rlat;
		tp->lon = rlon;
		priv->trip = g_slist_append(priv->trip, tp);
	}

	//Automatically center the map if the track approaches the edge
	if(priv->global_autocenter)	{
		int width = GTK_WIDGET(map)->allocation.width;
		int height = GTK_WIDGET(map)->allocation.height;
		if( x < (width/2 - width/8) 	|| x > (width/2 + width/8) 	|| 
			y < (height/2 - height/8) 	|| y > (height/2 + height/8)) {

			priv->global_x = pixel_x - GTK_WIDGET(map)->allocation.width/2;
			priv->global_y = pixel_y - GTK_WIDGET(map)->allocation.height/2;
		}
	}

	// dont draw anything if we are dragging
	if (priv->wtfcounter > 0) {
		g_debug("Dragging %d", priv->wtfcounter);
		return;
	}

	// this redraws the map (including the gps track, and adjusts the
	// map center if it was changed
	osm_gps_map_map_redraw(map);

	// recalculate x and y incase we have autocentered
	x = pixel_x - priv->global_x;
	y = pixel_y - priv->global_y;
	// draw current position as filled arc
	// done last so that only one arc is drawn
	marker = gdk_gc_new(priv->pixmap);
	color.red = 5000;
	color.green = 5000;
	color.blue = 55000;
	gdk_gc_set_rgb_fg_color(marker, &color);
    gdk_gc_set_line_attributes(marker,7, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
	gdk_draw_arc (priv->pixmap,
				  marker,
				  FALSE,		//filled
				  x-15, y-15,	// x,y
				  30,30,		// width, height
				  0, 360*64);	// start-end angle 64th, from 3h, anti clockwise
	gtk_widget_queue_draw_area (GTK_WIDGET(map),
								x-(15+7),y-(15+7),
								30+7+70,30+7+7);

}

void
osm_gps_map_clear_gps (OsmGpsMap *map)
{
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);
	if (priv->trip) {
		g_slist_foreach(priv->trip, (GFunc) g_free, NULL);
		g_slist_free(priv->trip);
		priv->trip = NULL;
		osm_gps_map_map_redraw (map);
	}
}

coord_t
osm_gps_map_get_co_ordinates (OsmGpsMap *map, int pixel_x, int pixel_y)
{
	coord_t coord;
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);

	coord.lat = rad2deg(pixel2lat(priv->global_zoom, priv->global_y + pixel_y));
	coord.lon = rad2deg(pixel2lon(priv->global_zoom, priv->global_x + pixel_x));
	return coord;
}

GtkWidget *
osm_gps_map_new (void)
{
	return g_object_new (OSM_TYPE_GPS_MAP, NULL);
}
