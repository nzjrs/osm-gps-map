/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*- */
/* vim:set et sw=4 ts=4 cino=t0,(0: */
/*
 * main.c
 * Copyright (C) John Stowers 2008 <john.stowers@gmail.com>
 *
 * This is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <math.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "osm-gps-map.h"

static OsmGpsMapSource_t opt_map_provider = OSM_GPS_MAP_SOURCE_OPENSTREETMAP;
static gboolean opt_default_cache = FALSE;
static gboolean opt_no_cache = FALSE;
static gboolean opt_debug = FALSE;
static char *opt_cache_base_dir = NULL;
static GOptionEntry entries[] =
{
  { "default-cache", 'D', 0, G_OPTION_ARG_NONE, &opt_default_cache, "Store maps using default cache style (md5)", NULL },
  { "no-cache", 'n', 0, G_OPTION_ARG_NONE, &opt_no_cache, "Disable cache", NULL },
  { "cache-basedir", 'b', 0, G_OPTION_ARG_FILENAME, &opt_cache_base_dir, "Cache basedir", NULL },
  { "debug", 'd', 0, G_OPTION_ARG_NONE, &opt_debug, "Enable debugging", NULL },
  { "map", 'm', 0, G_OPTION_ARG_INT, &opt_map_provider, "Map source", "N" },
  { NULL }
};

static GdkPixbuf *g_start_image = NULL;
static OsmGpsMapImage *g_last_image = NULL;

typedef struct {
    OsmGpsMap *map;
    GtkWidget *entry;
} timeout_cb_t;

#define DEG2RAD(deg) (deg * M_PI / 180.0)
#define RAD2DEG(rad) (rad / M_PI * 180.0)

gboolean
on_timeout_check_tiles_in_queue(gpointer user_data)
{
    gchar *msg;
    int remaining;
    timeout_cb_t *data = (timeout_cb_t *)user_data;
    g_object_get(data->map, "tiles-queued", &remaining,NULL);

    msg = g_strdup_printf("%d tiles queued",remaining);
    gtk_entry_set_text(GTK_ENTRY(data->entry), msg);
    g_free(msg);

    return remaining > 0;
}

gboolean
on_button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
    OsmGpsMapPoint coord;
    OsmGpsMap *map = OSM_GPS_MAP(widget);
    OsmGpsMapTrack *othertrack = OSM_GPS_MAP_TRACK(user_data);

    if (event->type == GDK_3BUTTON_PRESS) {
        if (event->button == 1) {
            if (g_last_image)
                osm_gps_map_image_remove (map, g_last_image);
        }
        if (event->button == 3) {
            osm_gps_map_track_remove(map, othertrack);
        }
    }

    if (event->type == GDK_2BUTTON_PRESS) {
        coord = osm_gps_map_get_co_ordinates(map, (int)event->x, (int)event->y);
        if (event->button == 1) {
            osm_gps_map_gps_add (map,
                                 RAD2DEG(coord.rlat),
                                 RAD2DEG(coord.rlon),
                                 g_random_double_range(0,360));
        }
        if (event->button == 3) {
            OsmGpsMapPoint *pt = g_boxed_copy(OSM_TYPE_GPS_MAP_POINT, &coord);
            osm_gps_map_track_add_point(othertrack, pt);
        }
    }

    if (event->type == GDK_BUTTON_PRESS) {
        if (event->button == 2) {
            coord = osm_gps_map_get_co_ordinates(map, (int)event->x, (int)event->y);
            g_last_image = osm_gps_map_image_add (
                                    map,
                                    RAD2DEG(coord.rlat),
                                    RAD2DEG(coord.rlon),
                                    g_start_image);
        }
    }
    return FALSE;
}

gboolean
on_button_release_event (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
    float lat,lon;
    GtkEntry *entry = GTK_ENTRY(user_data);
    OsmGpsMap *map = OSM_GPS_MAP(widget);

    g_object_get(map, "latitude", &lat, "longitude", &lon, NULL);
    gchar *msg = g_strdup_printf("%f,%f",lat,lon);
    gtk_entry_set_text(entry, msg);
    g_free(msg);

    return FALSE;
}

gboolean
on_zoom_in_clicked_event (GtkWidget *widget, gpointer user_data)
{
    int zoom;
    OsmGpsMap *map = OSM_GPS_MAP(user_data);
    g_object_get(map, "zoom", &zoom, NULL);
    osm_gps_map_set_zoom(map, zoom+1);
    return FALSE;
}

gboolean
on_zoom_out_clicked_event (GtkWidget *widget, gpointer user_data)
{
    int zoom;
    OsmGpsMap *map = OSM_GPS_MAP(user_data);
    g_object_get(map, "zoom", &zoom, NULL);
    osm_gps_map_set_zoom(map, zoom-1);
    return FALSE;
}

gboolean
on_home_clicked_event (GtkWidget *widget, gpointer user_data)
{
    OsmGpsMap *map = OSM_GPS_MAP(user_data);
    osm_gps_map_set_mapcenter(map, -43.5326,172.6362,12);
    return FALSE;
}

gboolean
on_cache_clicked_event (GtkWidget *widget, gpointer user_data)
{
    int zoom,max_zoom;
    OsmGpsMapPoint pt1, pt2;
    timeout_cb_t *data;

    data = (timeout_cb_t *)user_data;
    osm_gps_map_get_bbox(data->map, &pt1, &pt2);
    g_object_get(data->map, "zoom", &zoom, "max-zoom", &max_zoom, NULL);
    osm_gps_map_download_maps(data->map, &pt1, &pt2, zoom, max_zoom);
    g_timeout_add(500, on_timeout_check_tiles_in_queue, user_data);

    return FALSE;
}

void
on_close (GtkWidget *widget, gpointer user_data)
{
    gtk_widget_destroy(widget);
    gtk_main_quit();
}

void
usage (GOptionContext *context)
{
    int i;

    puts(g_option_context_get_help(context, TRUE, NULL));

    printf("Valid map sources:\n");
    for(i=OSM_GPS_MAP_SOURCE_NULL; i <= OSM_GPS_MAP_SOURCE_LAST; i++)
    {
        const char *name = osm_gps_map_source_get_friendly_name(i);
        const char *uri = osm_gps_map_source_get_repo_uri(i);
        if (uri != NULL)
            printf("\t%d:\t%s\n",i,name);
    }
}

int
main (int argc, char **argv)
{
    GtkWidget *vbox;
    GtkWidget *bbox;
    GtkWidget *entry;
    GtkWidget *window;
    GtkWidget *zoomInbutton;
    GtkWidget *zoomOutbutton;
    GtkWidget *homeButton;
    GtkWidget *cacheButton;
    OsmGpsMap *map;
    OsmGpsMapLayer *osd;
    OsmGpsMapTrack *rightclicktrack;
    const char *repo_uri;
    const char *friendly_name;
    char *cachedir, *cachebasedir;
    GError *error = NULL;
    GOptionContext *context;
    timeout_cb_t *data;

    g_thread_init(NULL);
    gtk_init (&argc, &argv);

    context = g_option_context_new ("- Map browser");
    g_option_context_set_help_enabled(context, FALSE);
    g_option_context_add_main_entries (context, entries, NULL);

    if (!g_option_context_parse (context, &argc, &argv, &error)) {
        usage(context);
        return 1;
    }

    /* Only use the repo_uri to check if the user has supplied a
    valid map source ID */
    repo_uri = osm_gps_map_source_get_repo_uri(opt_map_provider);
    if ( repo_uri == NULL ) {
        usage(context);
        return 2;
    }

    friendly_name = osm_gps_map_source_get_friendly_name(opt_map_provider);
    cachebasedir = osm_gps_map_get_default_cache_directory();

    if (opt_cache_base_dir && g_file_test(opt_cache_base_dir, G_FILE_TEST_IS_DIR)) {
        cachedir = g_strdup(OSM_GPS_MAP_CACHE_AUTO);
        cachebasedir = g_strdup(opt_cache_base_dir);
    } else if (opt_default_cache) {
        cachedir = g_strdup(OSM_GPS_MAP_CACHE_AUTO);
    } else if (opt_no_cache) {
        cachedir = g_strdup(OSM_GPS_MAP_CACHE_DISABLED);
    } else {
        cachedir = g_build_filename(cachebasedir,friendly_name,NULL);
    }

    if (opt_debug)
        gdk_window_set_debug_updates(TRUE);

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);

    g_start_image = gdk_pixbuf_new_from_file_at_size ("poi.png", 24,24,NULL);

    g_debug("Map Cache Dir: %s", cachedir);
    g_debug("Map Provider: %s (%d)", friendly_name, opt_map_provider);

    map = g_object_new (OSM_TYPE_GPS_MAP,
                        "map-source",opt_map_provider,
                        "tile-cache",cachedir,
                        "tile-cache-base", cachebasedir,
                        "proxy-uri",g_getenv("http_proxy"),
                        NULL);

    osd = g_object_new (OSM_TYPE_GPS_MAP_OSD,
                        "show-scale",TRUE,
                        "show-coordinates",TRUE,
                        "show-crosshair",TRUE,
                        "show-dpad",TRUE,
                        "show-zoom",TRUE,
                        "show-gps-in-dpad",TRUE,
                        "show-gps-in-zoom",FALSE,
                        "dpad-radius", 30,
                        NULL);
    osm_gps_map_add_layer(OSM_GPS_MAP(map), osd);
    g_object_unref(G_OBJECT(osd));

    //Add a second track for right clicks
    rightclicktrack = osm_gps_map_track_new();
    osm_gps_map_track_add(OSM_GPS_MAP(map), rightclicktrack);

    g_free(cachedir);
    g_free(cachebasedir);

    //Enable keyboard navigation
    osm_gps_map_set_keyboard_shortcut(map, OSM_GPS_MAP_KEY_FULLSCREEN, GDK_F11);
    osm_gps_map_set_keyboard_shortcut(map, OSM_GPS_MAP_KEY_UP, GDK_Up);
    osm_gps_map_set_keyboard_shortcut(map, OSM_GPS_MAP_KEY_DOWN, GDK_Down);
    osm_gps_map_set_keyboard_shortcut(map, OSM_GPS_MAP_KEY_LEFT, GDK_Left);
    osm_gps_map_set_keyboard_shortcut(map, OSM_GPS_MAP_KEY_RIGHT, GDK_Right);

    vbox = gtk_vbox_new (FALSE, 2);
    gtk_container_add (GTK_CONTAINER (window), vbox);

    //Add the map to the box
    gtk_box_pack_start (GTK_BOX(vbox), GTK_WIDGET(map), TRUE, TRUE, 0);
    //And add a box for the buttons
    bbox = gtk_hbox_new (TRUE, 0);
    gtk_box_pack_start (GTK_BOX(vbox), bbox, FALSE, TRUE, 0);
    //And add the lat/long entry
    entry = gtk_entry_new();
    gtk_box_pack_start (GTK_BOX(vbox), entry, FALSE, TRUE, 0);

    //Add buttons to the bbox
    zoomInbutton = gtk_button_new_from_stock (GTK_STOCK_ZOOM_IN);
    g_signal_connect (G_OBJECT (zoomInbutton), "clicked",
                      G_CALLBACK (on_zoom_in_clicked_event), (gpointer) map);
    gtk_box_pack_start (GTK_BOX(bbox), zoomInbutton, FALSE, TRUE, 0);

    zoomOutbutton = gtk_button_new_from_stock (GTK_STOCK_ZOOM_OUT);
    g_signal_connect (G_OBJECT (zoomOutbutton), "clicked",
                      G_CALLBACK (on_zoom_out_clicked_event), (gpointer) map);
    gtk_box_pack_start (GTK_BOX(bbox), zoomOutbutton, FALSE, TRUE, 0);

    homeButton = gtk_button_new_from_stock (GTK_STOCK_HOME);
    g_signal_connect (G_OBJECT (homeButton), "clicked",
                      G_CALLBACK (on_home_clicked_event), (gpointer) map);
    gtk_box_pack_start (GTK_BOX(bbox), homeButton, FALSE, TRUE, 0);

    data = g_new0(timeout_cb_t, 1);
    data->map = map;
    data->entry = entry;
    cacheButton = gtk_button_new_with_label ("Cache");
    g_signal_connect (G_OBJECT (cacheButton), "clicked",
                      G_CALLBACK (on_cache_clicked_event), (gpointer) data);
    gtk_box_pack_start (GTK_BOX(bbox), cacheButton, FALSE, TRUE, 0);

    //Connect to map events
    g_signal_connect (G_OBJECT (map), "button-press-event",
                      G_CALLBACK (on_button_press_event), (gpointer) rightclicktrack);
    g_signal_connect (G_OBJECT (map), "button-release-event",
                      G_CALLBACK (on_button_release_event), (gpointer) entry);

    g_signal_connect (window, "destroy",
                      G_CALLBACK (on_close), (gpointer) map);

    gtk_widget_show_all (window);

    g_log_set_handler ("OsmGpsMap", G_LOG_LEVEL_MASK, g_log_default_handler, NULL);
    gtk_main ();

    return 0;
}
