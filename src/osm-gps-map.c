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

#include "config.h"

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

#ifdef USE_CAIRO
#include <cairo.h>
#endif

#define ENABLE_DEBUG 0

typedef struct _OsmGpsMapPrivate OsmGpsMapPrivate;
struct _OsmGpsMapPrivate
{
	GHashTable *tile_queue;
	GHashTable *missing_tiles;

	int map_zoom;
	int max_zoom;
	int min_zoom;
	gboolean map_auto_center;
	gboolean map_auto_download;
	int map_x;
	int map_y;

	//how we download tiles
	SoupSession *soup_session;
	char *proxy_uri;

	//where downloaded tiles are cached	
	char *cache_dir;
	gboolean cache_dir_is_full_path;

	//contains flags indicating the various special characters
	//the uri string contains, that will be replaced when calculating
	//the uri to download.
	char *repo_uri;
	int uri_format;
	//flag indicating if the map source is located on the google
	gboolean the_google;

	//gps tracking state
	gboolean record_trip_history;
	gboolean show_trip_history;
	GSList *trip_history;
	coord_t *gps;
	gboolean gps_valid;

	//additional images or tracks added to the map
	GSList *tracks;
	GSList *images;

	//Used for storing the joined tiles
	GdkPixmap *pixmap;
	GdkGC *gc_map;

	//The tile painted when one cannot be found
	//GdkPixbuf *missing_tile;
	
	//For tracking click and drag
	int drag_counter;
	int drag_mouse_dx;
	int drag_mouse_dy;
	int	drag_start_mouse_x;
	int	drag_start_mouse_y;
	int drag_start_map_x;
	int drag_start_map_y;
};

#define OSM_GPS_MAP_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), OSM_TYPE_GPS_MAP, OsmGpsMapPrivate))

enum
{
	PROP_0,

	PROP_AUTO_CENTER,
	PROP_RECORD_TRIP_HISTORY,
	PROP_SHOW_TRIP_HISTORY,
	PROP_AUTO_DOWNLOAD,
	PROP_REPO_URI,
	PROP_PROXY_URI,
	PROP_TILE_CACHE_DIR,
	PROP_TILE_CACHE_DIR_IS_FULL_PATH,
	PROP_ZOOM,
	PROP_MAX_ZOOM,
	PROP_MIN_ZOOM,
	PROP_LATITUDE,
	PROP_LONGITUDE,
	PROP_MAP_X,
	PROP_MAP_Y,
	PROP_TILES_QUEUED
};

G_DEFINE_TYPE (OsmGpsMap, osm_gps_map, GTK_TYPE_DRAWING_AREA);

/* 
 * Drawing function forward defintions
 */
static gchar 	*replace_string(const gchar *src, const gchar *from, const gchar *to);
static void		inspect_map_uri(OsmGpsMap *map);
static gchar 	*replace_map_uri(OsmGpsMap *map, const gchar *uri, int zoom, int x, int y);
static void 	osm_gps_map_print_images (OsmGpsMap *map);
static void		osm_gps_map_draw_gps_point (OsmGpsMap *map);
static void		osm_gps_map_blit_tile(OsmGpsMap *map, GdkPixbuf *pixbuf, int offset_x, int offset_y);
static void		osm_gps_map_tile_download_complete (SoupSession *session, SoupMessage *msg, gpointer user_data);
static void		osm_gps_map_download_tile (OsmGpsMap *map, int zoom, int x, int y, int offset_x, int offset_y);
static void		osm_gps_map_load_tile (OsmGpsMap *map, int zoom, int x, int y, int offset_x, int offset_y);
static void		osm_gps_map_fill_tiles_pixel (OsmGpsMap *map, int pixel_x, int pixel_y, int zoom);
static void		osm_gps_map_map_redraw (OsmGpsMap *map);

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

static void
map_convert_coords_to_quadtree_string(OsmGpsMap *map, gint x, gint y, gint zoomlevel,
                                      gchar *buffer, const gchar initial,
                                      const gchar *const quadrant)
{
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);
    gchar *ptr = buffer;
    gint n;

    if (initial)
        *ptr++ = initial;

    for(n = zoomlevel-1; n >= 0; n--)
    {
        gint xbit = (x >> n) & 1;
        gint ybit = (y >> n) & 1;
        *ptr++ = quadrant[xbit + 2 * ybit];
    }

    *ptr++ = '\0';
}


static void
inspect_map_uri(OsmGpsMap *map)
{
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);

	if (g_strrstr(priv->repo_uri, URI_MARKER_X))
		priv->uri_format |= URI_HAS_X;

	if (g_strrstr(priv->repo_uri, URI_MARKER_Y))
		priv->uri_format |= URI_HAS_Y;

	if (g_strrstr(priv->repo_uri, URI_MARKER_Z))
		priv->uri_format |= URI_HAS_Z;

	if (g_strrstr(priv->repo_uri, URI_MARKER_S))
		priv->uri_format |= URI_HAS_S;

	if (g_strrstr(priv->repo_uri, URI_MARKER_Q))
		priv->uri_format |= URI_HAS_Q;	

	if (g_strrstr(priv->repo_uri, URI_MARKER_Q0))
		priv->uri_format |= URI_HAS_Q0;	

	if (g_strrstr(priv->repo_uri, URI_MARKER_YS))
		priv->uri_format |= URI_HAS_YS;	

	if (g_strrstr(priv->repo_uri, URI_MARKER_R))
		priv->uri_format |= URI_HAS_R;

	if (g_strrstr(priv->repo_uri, "google.com"))
		priv->the_google = TRUE;

	g_debug("URI Format: 0x%X (google: %X)", priv->uri_format, priv->the_google);

}

static gchar *
replace_map_uri(OsmGpsMap *map, const gchar *uri, int zoom, int x, int y)
{
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);
	char *url;
	unsigned int i;
	char location[22];

	i = 1;
	url = g_strdup(uri);
	while (i < URI_FLAG_END) 
	{
		char *s;
		char *old;

		old = url;
		switch(i & priv->uri_format) 
		{
			case URI_HAS_X:
				s = g_strdup_printf("%d", x);
				url = replace_string(url, URI_MARKER_X, s);
				//g_debug("FOUND " URI_MARKER_X);
				break;
			case URI_HAS_Y:
				s = g_strdup_printf("%d", y);
				url = replace_string(url, URI_MARKER_Y, s);
				//g_debug("FOUND " URI_MARKER_Y);
				break;
			case URI_HAS_Z:
				s = g_strdup_printf("%d", zoom);
				url = replace_string(url, URI_MARKER_Z, s);
				//g_debug("FOUND " URI_MARKER_Z);
				break;
			case URI_HAS_S:
				s = g_strdup_printf("%d", priv->max_zoom-zoom);
				url = replace_string(url, URI_MARKER_S, s);
				//g_debug("FOUND " URI_MARKER_S);
				break;
			case URI_HAS_Q:
				map_convert_coords_to_quadtree_string(map,x,y,zoom,location,'t',"qrts");
				s = g_strdup_printf("%s", location);
				url = replace_string(url, URI_MARKER_Q, s);
				//g_debug("FOUND " URI_MARKER_Q);
				break;
			case URI_HAS_Q0:
				map_convert_coords_to_quadtree_string(map,x,y,zoom,location,'\0', "0123");
				s = g_strdup_printf("%s", location);
				url = replace_string(url, URI_MARKER_Q0, s);
				//g_debug("FOUND " URI_MARKER_Q0);
				break;
			case URI_HAS_YS:
//				s = g_strdup_printf("%d", y);
//				url = replace_string(url, URI_MARKER_YS, s);
				g_warning("FOUND " URI_MARKER_YS " NOT IMPLEMENTED");
//            retval = g_strdup_printf(repo->url,
//                    tilex,
//                    (1 << (MAX_ZOOM - zoom)) - tiley - 1,
//                    zoom - (MAX_ZOOM - 17));
            	break;
			case URI_HAS_R:
				s = g_strdup_printf("%d", g_random_int_range(0,4));
				url = replace_string(url, URI_MARKER_R, s);
				//g_debug("FOUND " URI_MARKER_R);
				break;
			default:
				s = NULL;
				break;
		}

		if (s) {
			g_free(s);
			g_free(old);			
		}

		i = (i << 1);

	}

	return url;
}

static void
my_log_handler (const gchar * log_domain, GLogLevelFlags log_level, const gchar * message, gpointer user_data)
{
	if (!(log_level & G_LOG_LEVEL_DEBUG) || ENABLE_DEBUG)
		g_log_default_handler (log_domain, log_level, message, user_data);
}

/* clears the trip list and all resources */
static void
osm_gps_map_free_trip (OsmGpsMap *map)
{
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);
	if (priv->trip_history) {
		g_slist_foreach(priv->trip_history, (GFunc) g_free, NULL);
		g_slist_free(priv->trip_history);
		priv->trip_history = NULL;
	}
}

/* clears the tracks and all resources */
static void
osm_gps_map_free_tracks (OsmGpsMap *map)
{
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);
	if (priv->tracks) 
	{
		GSList* tmp = priv->tracks;
		while (tmp != NULL)
	  	{
			g_slist_foreach(tmp->data, (GFunc) g_free, NULL);
			g_slist_free(tmp->data);
			tmp = g_slist_next(tmp);
		}
		g_slist_free(priv->tracks);
		priv->tracks = NULL;
	}
}

/* free the poi image lists */
static void
osm_gps_map_free_images (OsmGpsMap *map)
{
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);
	if (priv->images) {
		GSList *list;
		for(list = priv->images; list != NULL; list = list->next)
		{
			image_t *im = list->data;
			g_object_unref(im->image);
			g_free(im);
		}
		g_slist_free(priv->images);
		priv->images = NULL;
	}
}

static void
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
		pixel_x = lon2pixel(priv->map_zoom, im->pt.rlon);
		pixel_y = lat2pixel(priv->map_zoom, im->pt.rlat);
		
		g_debug("Image %dx%d @: %f,%f (%d,%d)",
					im->w, im->h,
					im->pt.rlat, im->pt.rlon,
					pixel_x, pixel_y);
		
		x = pixel_x - priv->map_x;
		y = pixel_y - priv->map_y;
		
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

static void 
osm_gps_map_draw_gps_point (OsmGpsMap *map)
{
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);

	//incase we get called before we have got a gps point
	if (priv->gps_valid) {
		int x,y;
#ifdef USE_CAIRO
		cairo_t *cr = gdk_cairo_create(priv->pixmap);
		int r = TILE_POINT_SIZE;
		cairo_pattern_t *pat;
#else
		GdkColor color;
		GdkGC *marker;
#endif

		x = lon2pixel(priv->map_zoom, priv->gps->rlon) - priv->map_x;
		y = lat2pixel(priv->map_zoom, priv->gps->rlat) - priv->map_y;

#ifdef USE_CAIRO
		// draw ball gradient
		pat = cairo_pattern_create_radial (x-(r/5), y-(r/5), (r/5), x,  y, r);
		cairo_pattern_add_color_stop_rgba (pat, 0, 1, 1, 1, 1.0);
		cairo_pattern_add_color_stop_rgba (pat, 1, 0, 0, 1, 1.0);
		cairo_set_source (cr, pat);
		cairo_arc (cr, x, y, r, 0, 2 * M_PI);
		cairo_fill (cr);
		cairo_pattern_destroy (pat);
		// draw ball border
		cairo_set_line_width (cr, 1.0);
		cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1.0);
		cairo_arc (cr, x, y, r, 0, 2 * M_PI);
		cairo_stroke(cr);
		cairo_destroy(cr);

		gtk_widget_queue_draw_area (GTK_WIDGET(map), x-r, y-r, r*2, r*2);
#else
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
		g_object_unref(marker);

		gtk_widget_queue_draw_area (GTK_WIDGET(map),
									x-(15+7),y-(15+7),
									30+7+7,30+7+7);
#endif
	}
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
			if (fd != -1) 
			{
				write (fd, msg->response_body->data, msg->response_body->length);
				g_debug("Wrote %lld bytes to %s", msg->response_body->length, dl->filename);
				close (fd);
				/* Redraw the area of the screen */
				if ( msg->response_body->length > 0 ) 
				{
					/* Unless these tiles were explicityly just queued, and 
					should not be redrawn */
					if (dl->offset_x != G_MININT && dl->offset_y != G_MININT) 
					{
						GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file (dl->filename, NULL);
						if(pixbuf) 
						{
							g_debug("Found tile %s", dl->filename);
							osm_gps_map_blit_tile(map, pixbuf, dl->offset_x,dl->offset_y);
							g_object_unref (pixbuf);
						}
					}
				}
			}
		}
		else 
		{
			g_warning("Error creating tile download directory: %s", dl->folder);
		}

		g_hash_table_remove(priv->tile_queue, dl->uri);

		//if we finished downloading all tiles then we need to redraw the line
		if (g_hash_table_size(priv->tile_queue) == 0)
			osm_gps_map_map_redraw(map);

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
			;//application exiting
		}
		else 
		{
			soup_session_requeue_message(session, msg);
			return;
		}
	}


}

static void
osm_gps_map_download_tile (OsmGpsMap *map, int zoom, int x, int y, int offset_x, int offset_y)
{
	SoupMessage *msg;
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);
	tile_download_t *dl = g_new0(tile_download_t,1);

	//calculate the uri to download	
	dl->uri = replace_map_uri(map, priv->repo_uri, zoom, x, y);
		
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
			if (priv->the_google) {
				//Set maps.google.com as the referrer
				g_debug("Setting Google Referrer");
				soup_message_headers_append(msg->request_headers, "Referer", "http://maps.google.com/");
				//For google satelite also set the appropriate cookie value
				if (priv->uri_format & URI_HAS_Q) {
					const char *cookie = g_getenv("GOOGLE_COOKIE");
					if (cookie) {
						g_debug("Adding Google Cookie");
						soup_message_headers_append(msg->request_headers, "Cookie", cookie);
					}
				}
			}

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
		if (priv->map_auto_download)
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
	
	offset_x = - pixel_x % TILESIZE;
	offset_y = - pixel_y % TILESIZE;
	if (offset_x > 0) offset_x -= 256;
	if (offset_y > 0) offset_y -= 256;
	
	offset_xn = offset_x;
	offset_yn = offset_y;

	width  = GTK_WIDGET(map)->allocation.width;
	height = GTK_WIDGET(map)->allocation.height;

	tiles_nx = floor((width  - offset_x) / TILESIZE) + 1;
	tiles_ny = floor((height - offset_y) / TILESIZE) + 1;
	
	tile_x0 =  floor((float)pixel_x / (float)TILESIZE);
	tile_y0 =  floor((float)pixel_y / (float)TILESIZE);

	//TODO: implement wrap around
	for (i=tile_x0; i<(tile_x0+tiles_nx);i++)
	{
		for (j=tile_y0;  j<(tile_y0+tiles_ny); j++)
		{
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
osm_gps_map_print_track (OsmGpsMap *map, GSList *trackpoint_list)
{
	GSList *list;
	int x,y;
	int min_x = 0,min_y = 0,max_x = 0,max_y = 0;
#ifdef USE_CAIRO
	cairo_t *cr;
#else
	int last_x = 0, last_y = 0;
	GdkColor color;
	GdkGC *gc;
#endif

	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);

#ifdef USE_CAIRO
	cr = gdk_cairo_create(priv->pixmap);
	cairo_set_line_width (cr, TILE_LINE_SIZE);
	cairo_set_source_rgba (cr, 0.2, 0.2, 1.0, 0.6);
	cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
	cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);
#else
	gc = gdk_gc_new(priv->pixmap);
	color.green = 0;
	color.blue = 0;
	color.red = 60000;
	gdk_gc_set_rgb_fg_color(gc, &color);
	gdk_gc_set_line_attributes(gc, 5, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_ROUND);
#endif

	for(list = trackpoint_list; list != NULL; list = list->next)
	{
		coord_t *tp = list->data;

		x = lon2pixel(priv->map_zoom, tp->rlon) - priv->map_x;
		y = lat2pixel(priv->map_zoom, tp->rlat) - priv->map_y;
	
		// first time through loop
		if (list == trackpoint_list) {
#ifdef USE_CAIRO
			cairo_move_to(cr, x, y);
#else
			last_x = x;
			last_y = y;
#endif
		}

#ifdef USE_CAIRO
		cairo_line_to(cr, x, y);
#else
		gdk_draw_line (priv->pixmap, gc, x, y, last_x, last_y);
		last_x = x;
		last_y = y;
#endif
		
		max_x = MAX(x,max_x);
		min_x = MIN(x,min_x);
		max_y = MAX(y,max_y);
		min_y = MIN(y,min_y);
	}

#ifdef USE_CAIRO
	cairo_stroke(cr);
	cairo_destroy(cr);

	gtk_widget_queue_draw_area (
			GTK_WIDGET(map),
			min_x - TILE_LINE_SIZE,
			min_y - TILE_LINE_SIZE,
			max_x + (TILE_LINE_SIZE * 2),
			max_y + (TILE_LINE_SIZE * 2));
#else	
	gtk_widget_queue_draw_area (
			GTK_WIDGET(map), 
			min_x-5, min_y-5,
			max_x+10, max_y+10);
	g_object_unref(gc);
#endif
}

/* Prints the gps trip history, and any other tracks */
static void
osm_gps_map_print_tracks (OsmGpsMap *map)
{
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);
	
	if (priv->show_trip_history)
		osm_gps_map_print_track (map, priv->trip_history);
	if (priv->tracks) 
	{
		g_debug("TRACK");
		GSList* tmp = priv->tracks;
		while (tmp != NULL)
	  	{
			osm_gps_map_print_track (map, tmp->data);
			tmp = g_slist_next(tmp);
		}
	}
}

static void
osm_gps_map_map_redraw (OsmGpsMap *map)
{
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);

	/* draw white background to initialise pixmap */
	gdk_draw_rectangle (
		priv->pixmap,
		GTK_WIDGET(map)->style->white_gc,
		TRUE,
		0, 0,
		GTK_WIDGET(map)->allocation.width+260,
		GTK_WIDGET(map)->allocation.height+260);

	osm_gps_map_fill_tiles_pixel(
		map,
		priv->map_x,
		priv->map_y,
		priv->map_zoom);

	osm_gps_map_print_tracks(map);
	osm_gps_map_draw_gps_point(map);
	osm_gps_map_print_images(map);

	//osm_gps_map_osd_speed(map, 1.5);
}

static void
osm_gps_map_init (OsmGpsMap *object)
{
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(object);

	priv->pixmap = NULL;

	priv->trip_history = NULL;
	priv->gps = g_new0(coord_t, 1);
	priv->gps_valid = FALSE;

	priv->tracks = NULL;
	priv->images = NULL;
	
	priv->drag_counter = 0;
	priv->drag_mouse_dx = 0;
	priv->drag_mouse_dy = 0;
	priv->drag_start_mouse_x = 0;
	priv->drag_start_mouse_y = 0;

	priv->uri_format = 0;
	priv->the_google = FALSE;

	//Change naumber of concurrent connections option?
	priv->soup_session = soup_session_async_new_with_options(
								SOUP_SESSION_USER_AGENT,
								 "Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.8.1.11) Gecko/20071127 Firefox/2.0.0.11",
								 NULL);

	//Hash table which maps tile d/l URIs to SoupMessage requests
	priv->tile_queue = g_hash_table_new (g_str_hash, g_str_equal);

	//Some mapping providers (Google) have varying degrees of tiles at multiple
	//zoom levels
	priv->missing_tiles = g_hash_table_new (g_str_hash, g_str_equal);

	gtk_widget_add_events (GTK_WIDGET (object),
			GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK);

  	g_log_set_handler (G_LOG_DOMAIN, G_LOG_LEVEL_MASK, my_log_handler, NULL);
}

static GObject *
osm_gps_map_constructor (GType gtype, guint n_properties, GObjectConstructParam *properties)
{
	GObject *object;
	OsmGpsMapPrivate *priv;

	//Always chain up to the parent constructor
	object = G_OBJECT_CLASS(osm_gps_map_parent_class)->constructor(gtype, n_properties, properties);
	priv = OSM_GPS_MAP_PRIVATE(object);

	if (!priv->cache_dir_is_full_path) {
		char *md5 = g_compute_checksum_for_string (G_CHECKSUM_MD5, priv->repo_uri, -1);

		if (priv->cache_dir) {
			char *old = priv->cache_dir;
			//the new cachedir is the given cache dir + the md5 of the repo_uri
			priv->cache_dir = g_strdup_printf("%s/%s", old, md5);
			g_debug("Adjusting cache dir %s -> %s", old, priv->cache_dir);
			g_free(old);
		} else {
			//the new cachedir is the current dir + the md5 of the repo_uri
			priv->cache_dir = g_strdup(md5);
		}
		
		g_free(md5);
	}	

	return object;
}

static void
osm_gps_map_finalize (GObject *object)
{
	OsmGpsMap *map = OSM_GPS_MAP(object);
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(object);

	soup_session_abort(priv->soup_session);
	g_object_unref(priv->soup_session);

	g_hash_table_destroy(priv->tile_queue);
	g_hash_table_destroy(priv->missing_tiles);

	g_free(priv->cache_dir);
	g_free(priv->repo_uri);

	osm_gps_map_free_trip(map);
	osm_gps_map_free_tracks(map);
	osm_gps_map_free_images(map);

	if(priv->pixmap)
		g_object_unref (priv->pixmap);

	if(priv->gc_map)
		g_object_unref(priv->gc_map);

	G_OBJECT_CLASS (osm_gps_map_parent_class)->finalize (object);
}

static void
osm_gps_map_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	g_return_if_fail (OSM_IS_GPS_MAP (object));
	OsmGpsMap *map = OSM_GPS_MAP(object);
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(object);

	switch (prop_id)
	{
	case PROP_AUTO_CENTER:
		priv->map_auto_center = g_value_get_boolean (value);
		break;
	case PROP_RECORD_TRIP_HISTORY:
		priv->record_trip_history = g_value_get_boolean (value);
		break;
	case PROP_SHOW_TRIP_HISTORY:
		priv->show_trip_history = g_value_get_boolean (value);
		break;
	case PROP_AUTO_DOWNLOAD:
		priv->map_auto_download = g_value_get_boolean (value);
		break;
	case PROP_REPO_URI:
		priv->repo_uri = g_value_dup_string (value);
		inspect_map_uri(map);
		break;
	case PROP_PROXY_URI:
		if ( g_value_get_string(value) ) {
			GValue val = {0};

			priv->proxy_uri = g_value_dup_string (value);
			g_debug("Setting proxy server: %s", priv->proxy_uri);

			SoupURI* uri = soup_uri_new(priv->proxy_uri);
			g_value_init(&val, SOUP_TYPE_URI);
			g_value_take_boxed(&val, uri);

			g_object_set_property(G_OBJECT(priv->soup_session),SOUP_SESSION_PROXY_URI,&val);
		} else
			priv->proxy_uri = NULL;

		break;
	case PROP_TILE_CACHE_DIR:
		priv->cache_dir = g_value_dup_string (value);
		break;
	case PROP_TILE_CACHE_DIR_IS_FULL_PATH:
		priv->cache_dir_is_full_path = g_value_get_boolean (value);
		break;
	case PROP_ZOOM:
		priv->map_zoom = g_value_get_int (value);
		break;
	case PROP_MAX_ZOOM:
		priv->max_zoom = g_value_get_int (value);
		break;
	case PROP_MIN_ZOOM:
		priv->min_zoom = g_value_get_int (value);
		break;
	case PROP_MAP_X:
		priv->map_x = g_value_get_int (value);
		break;
	case PROP_MAP_Y:
		priv->map_y = g_value_get_int (value);
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
		g_value_set_boolean(value, priv->map_auto_center);
		break;
	case PROP_RECORD_TRIP_HISTORY:
		g_value_set_boolean(value, priv->record_trip_history);
		break;
	case PROP_SHOW_TRIP_HISTORY:
		g_value_set_boolean(value, priv->show_trip_history);
		break;
	case PROP_AUTO_DOWNLOAD:
		g_value_set_boolean(value, priv->map_auto_download);
		break;
	case PROP_REPO_URI:
		g_value_set_string(value, priv->repo_uri);
		break;
	case PROP_PROXY_URI:
		g_value_set_string(value, priv->proxy_uri);
		break;
	case PROP_TILE_CACHE_DIR:
		g_value_set_string(value, priv->cache_dir);
		break;
	case PROP_TILE_CACHE_DIR_IS_FULL_PATH:
		g_value_set_boolean(value, priv->cache_dir_is_full_path);
		break;
	case PROP_ZOOM:
		g_value_set_int(value, priv->map_zoom);
		break;
	case PROP_MAX_ZOOM:
		g_value_set_int(value, priv->max_zoom);
		break;
	case PROP_MIN_ZOOM:
		g_value_set_int(value, priv->min_zoom);
		break;
	case PROP_LATITUDE:
		lat = pixel2lat(priv->map_zoom,
						priv->map_y + (GTK_WIDGET(map)->allocation.height / 2));
		g_value_set_float(value, rad2deg(lat));
		break;
	case PROP_LONGITUDE:
		lon = pixel2lon(priv->map_zoom,
						priv->map_x + (GTK_WIDGET(map)->allocation.width / 2));
		g_value_set_float(value, rad2deg(lon));
		break;
	case PROP_MAP_X:
		g_value_set_int(value, priv->map_x);
		break;
	case PROP_MAP_Y:
		g_value_set_int(value, priv->map_y);
		break;
	case PROP_TILES_QUEUED:
		g_value_set_int(value, g_hash_table_size(priv->tile_queue));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static gboolean
osm_gps_map_scroll (GtkWidget *widget, GdkEventScroll  *event)
{
	OsmGpsMap *map = OSM_GPS_MAP(widget);
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(widget);

	if (event->direction == GDK_SCROLL_UP)
	{
		osm_gps_map_set_zoom(map, priv->map_zoom+1);
	}
	else
	{
		osm_gps_map_set_zoom(map, priv->map_zoom-1);
	}

	return FALSE;
}

static gboolean
osm_gps_map_button_press (GtkWidget *widget, GdkEventButton *event)
{
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(widget);

	priv->drag_counter = 0;
	priv->drag_start_mouse_x = (int) event->x;
	priv->drag_start_mouse_y = (int) event->y;
	priv->drag_start_map_x = priv->map_x;
	priv->drag_start_map_y = priv->map_y;
	
	return FALSE;
}

static gboolean
osm_gps_map_button_release (GtkWidget *widget, GdkEventButton *event)
{
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(widget);

	if(priv->drag_counter >= 6)
	{
		priv->map_x = priv->drag_start_map_x;
		priv->map_y = priv->drag_start_map_y;
		
		priv->map_x += (priv->drag_start_mouse_x - (int) event->x);
		priv->map_y += (priv->drag_start_mouse_y - (int) event->y);
	
		osm_gps_map_map_redraw(OSM_GPS_MAP(widget));
	}

	priv->drag_mouse_dx = 0;
	priv->drag_mouse_dy = 0;
	priv->drag_counter = 0;

	return FALSE;
}

static gboolean
osm_gps_map_motion_notify (GtkWidget *widget, GdkEventMotion  *event)
{
	int x, y, width, height;
	GdkModifierType state;
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(widget);
	
	width  = widget->allocation.width;
	height = widget->allocation.height;
	
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
		if (priv->drag_counter >= 6) {

			if (priv->map_auto_center)
				g_object_set(G_OBJECT(widget), "auto-center", FALSE, NULL);

			priv->drag_mouse_dx = x - priv->drag_start_mouse_x;	
			priv->drag_mouse_dy = y - priv->drag_start_mouse_y;

			gdk_draw_drawable (
				widget->window,
				widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
				priv->pixmap,
				0,0,
				priv->drag_mouse_dx,priv->drag_mouse_dy,
				-1,-1);

			//Paint white to the top and left of the map if dragging. Its less
			//ugly than painting the corrupted map		
			if(priv->drag_mouse_dx>0) {
				gdk_draw_rectangle (
					widget->window,
					widget->style->white_gc,
					TRUE,
					0, 0,
					priv->drag_mouse_dx,
					widget->allocation.height);
			}
		
			if (priv->drag_mouse_dy>0) {
				gdk_draw_rectangle (
					widget->window,
					widget->style->white_gc,
					TRUE,
					0, 0,
					widget->allocation.width,
					priv->drag_mouse_dy);
			}
			//g_debug("Motion: %i %i - start: %i %i - dx: %i %i --wtf %i\n", x,y, priv->drag_start_mouse_x, priv->drag_start_mouse_y, priv->drag_mouse_dx, priv->drag_mouse_dy, priv->drag_counter);
		} else {
			priv->drag_counter++;
		}
	}

	return FALSE;
}

static gboolean
osm_gps_map_configure (GtkWidget *widget, GdkEventConfigure *event)
{
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(widget);

	/* create pixmap */
	if (priv->pixmap)
		g_object_unref (priv->pixmap);

	priv->pixmap = gdk_pixmap_new (
			widget->window,
			widget->allocation.width+260,
			widget->allocation.height+260,
			-1);

	/* and gc, used for clipping (I think......) */
	if(priv->gc_map)
		g_object_unref(priv->gc_map);

	priv->gc_map = gdk_gc_new(priv->pixmap);

	osm_gps_map_map_redraw(OSM_GPS_MAP(widget));

	return FALSE;
}

static gboolean
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
osm_gps_map_class_init (OsmGpsMapClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	g_type_class_add_private (klass, sizeof (OsmGpsMapPrivate));

	object_class->finalize = osm_gps_map_finalize;
	object_class->constructor = osm_gps_map_constructor;
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
	                                 PROP_RECORD_TRIP_HISTORY,
	                                 g_param_spec_boolean ("record-trip-history",
	                                                       "record trip history",
	                                                       "should all gps points be recorded in a trip history",
	                                                       TRUE,
	                                                       G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT));

	g_object_class_install_property (object_class,
	                                 PROP_SHOW_TRIP_HISTORY,
	                                 g_param_spec_boolean ("show-trip-history",
	                                                       "show trip history",
	                                                       "should the recorded trip history be shown on the map",
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
	                                                      G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property (object_class,
	                                 PROP_PROXY_URI,
	                                 g_param_spec_string ("proxy-uri",
	                                                      "proxy uri",
	                                                      "http proxy uri on NULL",
	                                                      NULL,
	                                                      G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property (object_class,
	                                 PROP_TILE_CACHE_DIR,
	                                 g_param_spec_string ("tile-cache",
	                                                      "tile cache",
	                                                      "osm local tile cache dir",
	                                                      "/tmp/Maps",
	                                                      G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property (object_class,
	                                 PROP_TILE_CACHE_DIR_IS_FULL_PATH,
	                                 g_param_spec_boolean ("tile-cache-is-full-path",
	                                                       "tile cache is full path",
	                                                       "if true, the path passed to tile-cache is interpreted as the full cache path",
	                                                       FALSE,
	                                                       G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property (object_class,
	                                 PROP_ZOOM,
	                                 g_param_spec_int ("zoom",
	                                                    "zoom",
	                                                    "zoom level",
	                                                    MIN_ZOOM, /* minimum property value */
	                                                    MAX_ZOOM, /* maximum property value */
	                                                    3,
	                                                    G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property (object_class,
	                                 PROP_MAX_ZOOM,
	                                 g_param_spec_int ("max-zoom",
	                                                    "max zoom",
	                                                    "maximum zoom level",
	                                                    MIN_ZOOM, /* minimum property value */
	                                                    MAX_ZOOM, /* maximum property value */
	                                                    17,
	                                                    G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

	g_object_class_install_property (object_class,
	                                 PROP_MIN_ZOOM,
	                                 g_param_spec_int ("min-zoom",
	                                                    "min zoom",
	                                                    "minimum zoom level",
	                                                    MIN_ZOOM, /* minimum property value */
	                                                    MAX_ZOOM, /* maximum property value */
	                                                    1,
	                                                    G_PARAM_READABLE | G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));

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
	int i,j,zoom,num_tiles;
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);
	
	if (pt1 && pt2) 
	{
		gchar *filename;
		num_tiles = 0;
		zoom_end = CLAMP(zoom_end, priv->min_zoom, priv->max_zoom);
		g_debug("Download maps: z:%d->%d",zoom_start, zoom_end);
	
		for(zoom=zoom_start; zoom<=zoom_end; zoom++)
		{
			int x1,y1,x2,y2;

			x1 = (int)floor((float)lon2pixel(zoom, pt1->rlon) / (float)TILESIZE);
			y1 = (int)floor((float)lat2pixel(zoom, pt1->rlat) / (float)TILESIZE);

			x2 = (int)floor((float)lon2pixel(zoom, pt2->rlon) / (float)TILESIZE);
			y2 = (int)floor((float)lat2pixel(zoom, pt2->rlat) / (float)TILESIZE);

			// loop x1-x2
			for(i=x1; i<=x2; i++)
			{
				// loop y1 - y2
				for(j=y1; j<=y2; j++)
				{
					// x = i, y = j
					filename = g_strdup_printf("%s/%u/%u/%u.png", priv->cache_dir, zoom, i, j);
					if (!g_file_test(filename, G_FILE_TEST_EXISTS))
					{
						//Use G_MININT to signify the tiles should not be blitted when complete
						osm_gps_map_download_tile(map, zoom, i, j, G_MININT, G_MININT);
						num_tiles++;
					}
					g_free(filename);
				}
			}
			g_debug("DL @Z:%d = %d tiles",zoom,num_tiles);
		}
	}
}

void
osm_gps_map_get_bbox (OsmGpsMap *map, coord_t *pt1, coord_t *pt2)
{
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);
	
	if (pt1 && pt2) {
		pt1->rlat = pixel2lat(priv->map_zoom, priv->map_y);
		pt1->rlon = pixel2lon(priv->map_zoom, priv->map_x);
		pt2->rlat = pixel2lat(priv->map_zoom, priv->map_y + GTK_WIDGET(map)->allocation.height);
		pt2->rlon = pixel2lon(priv->map_zoom, priv->map_x + GTK_WIDGET(map)->allocation.width);

		g_debug("BBOX: %f %f %f %f", pt1->rlat, pt1->rlon, pt2->rlat, pt2->rlon);
	}
}

void
osm_gps_map_set_mapcenter (OsmGpsMap *map, float latitude, float longitude, int zoom)
{
	int pixel_x, pixel_y;
	float rlat, rlon;
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);

	//constrain zoom min_zoom -> max_zoom
	priv->map_zoom = CLAMP(zoom, priv->min_zoom, priv->max_zoom);
	
	rlat = deg2rad(latitude);
	rlon = deg2rad(longitude);
	
	// pixel_x,y, offsets
	pixel_x = lon2pixel(priv->map_zoom, rlon);
	pixel_y = lat2pixel(priv->map_zoom, rlat);

	priv->map_x = pixel_x - GTK_WIDGET(map)->allocation.width/2;
	priv->map_y = pixel_y - GTK_WIDGET(map)->allocation.height/2;

	osm_gps_map_map_redraw(map);
}

int 
osm_gps_map_set_zoom (OsmGpsMap *map, int zoom)
{
	int zoom_old;
	double factor;
	int width_center, height_center;
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);

	if (zoom != priv->map_zoom)
	{	
		width_center  = GTK_WIDGET(map)->allocation.width / 2;
		height_center = GTK_WIDGET(map)->allocation.height / 2;
		
		zoom_old = priv->map_zoom;
		//constrain zoom min_zoom -> max_zoom
		priv->map_zoom = CLAMP(zoom, priv->min_zoom, priv->max_zoom); 
		factor = exp(priv->map_zoom * M_LN2)/exp(zoom_old * M_LN2);
		
		g_debug("Zoom changed from %d to %d factor:%f x:%d", zoom_old,priv->map_zoom, factor, priv->map_x);
	
		priv->map_x = ((priv->map_x + width_center) * factor) - width_center;
		priv->map_y = ((priv->map_y + height_center) * factor) - height_center;
		
		osm_gps_map_map_redraw(map);
	}
	return priv->map_zoom;
}

void
osm_gps_map_add_track (OsmGpsMap *map, GSList *track)
{
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);
	if (track) {
		priv->tracks = g_slist_append(priv->tracks, track);
		osm_gps_map_map_redraw(map);
	}
}

void
osm_gps_map_clear_tracks (OsmGpsMap *map)
{
	osm_gps_map_free_tracks(map);
	osm_gps_map_map_redraw(map);
}

void
osm_gps_map_add_image (OsmGpsMap *map, float latitude, float longitude, GdkPixbuf *image)
{
	image_t *im;
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);
	
	if (image) {
		//cache w/h for speed, and add image to list
		im = g_new0(image_t,1);
		im->w = gdk_pixbuf_get_width(image);
		im->h = gdk_pixbuf_get_width(image);
		im->pt.rlat = deg2rad(latitude);
		im->pt.rlon = deg2rad(longitude);

		g_object_ref(image);
		im->image = image;
	
		priv->images = g_slist_append(priv->images, im);
		
		osm_gps_map_map_redraw(map);
	}
}

void
osm_gps_map_clear_images (OsmGpsMap *map)
{
	osm_gps_map_free_images(map);
	osm_gps_map_map_redraw(map);
}

void
osm_gps_map_osd_speed (OsmGpsMap *map, float speed)
{
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);

	PangoContext		*context = NULL;
	PangoLayout		*layout  = NULL;
	PangoFontDescription	*desc    = NULL;
	
	GdkColor color;
	GdkGC *gc;
	
	gchar *buffer;
	//static int x = 10, y = 10;
	static int width = 0, height = 0;

	buffer = g_strdup_printf("%.0f", speed);
	
	/* pango initialisation */
	context = gtk_widget_get_pango_context (GTK_WIDGET(map));
	layout  = pango_layout_new (context);
	desc    = pango_font_description_new();
	
	pango_font_description_set_size (desc, 40 * PANGO_SCALE);
	pango_layout_set_font_description (layout, desc);
	pango_layout_set_text (layout, buffer, strlen(buffer));

	gc = gdk_gc_new (GTK_WIDGET(map)->window);

	color.red = (0 > 50) ? 0xffff : 0;
	color.green = 0;
	color.blue = 0;
	
	gdk_gc_set_rgb_fg_color (gc, &color);

	/* faster / less flicker alternative:*/
	gdk_draw_drawable (
		GTK_WIDGET(map)->window,
		GTK_WIDGET(map)->style->fg_gc[GTK_WIDGET_STATE(map)],
		priv->pixmap,
		0,0,
		0,0,
		width+10,width+10);

	gdk_draw_layout(GTK_WIDGET(map)->window,
			gc,
			0, 0,
			layout);
	
	/* set width and height */
	pango_layout_get_pixel_size(layout, &width, &height);

	g_free(buffer);
	pango_font_description_free (desc);
	g_object_unref (layout);
	g_object_unref (gc);
}

void
osm_gps_map_draw_gps (OsmGpsMap *map, float latitude, float longitude, float heading)
{
	int pixel_x, pixel_y;
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);

	priv->gps->rlat = deg2rad(latitude);
	priv->gps->rlon = deg2rad(longitude);
	priv->gps_valid = TRUE;

	// pixel_x,y, offsets
	pixel_x = lon2pixel(priv->map_zoom, priv->gps->rlon);
	pixel_y = lat2pixel(priv->map_zoom, priv->gps->rlat);

	//If trip marker add to list of gps points.
	if (priv->record_trip_history) {
		coord_t *tp = g_new0(coord_t,1);
		tp->rlat = priv->gps->rlat;
		tp->rlon = priv->gps->rlon;
		priv->trip_history = g_slist_append(priv->trip_history, tp);
	}

	// dont draw anything if we are dragging
	if (priv->drag_counter > 0) {
		g_debug("Dragging %d", priv->drag_counter);
		return;
	}

	//Automatically center the map if the track approaches the edge
	if(priv->map_auto_center)	{
		int x = pixel_x - priv->map_x;
		int y = pixel_y - priv->map_y;
		int width = GTK_WIDGET(map)->allocation.width;
		int height = GTK_WIDGET(map)->allocation.height;
		if( x < (width/2 - width/8) 	|| x > (width/2 + width/8) 	|| 
			y < (height/2 - height/8) 	|| y > (height/2 + height/8)) {

			priv->map_x = pixel_x - GTK_WIDGET(map)->allocation.width/2;
			priv->map_y = pixel_y - GTK_WIDGET(map)->allocation.height/2;
		}
	}

	// this redraws the map (including the gps track, and adjusts the
	// map center if it was changed
	osm_gps_map_map_redraw(map);
}

void
osm_gps_map_clear_gps (OsmGpsMap *map)
{
	osm_gps_map_free_trip(map);
	osm_gps_map_map_redraw(map);
}

coord_t
osm_gps_map_get_co_ordinates (OsmGpsMap *map, int pixel_x, int pixel_y)
{
	coord_t coord;
	OsmGpsMapPrivate *priv = OSM_GPS_MAP_PRIVATE(map);

	coord.rlat = pixel2lat(priv->map_zoom, priv->map_y + pixel_y);
	coord.rlon = pixel2lon(priv->map_zoom, priv->map_x + pixel_x);
	return coord;
}

GtkWidget *
osm_gps_map_new (void)
{
	return g_object_new (OSM_TYPE_GPS_MAP, NULL);
}

