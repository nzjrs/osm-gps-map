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
static gboolean opt_friendly_cache = FALSE;
static gboolean opt_no_cache = FALSE;
static gboolean opt_debug = FALSE;
static char *opt_cache_base_dir = NULL;
static gboolean opt_editable_tracks = FALSE;
static GOptionEntry entries[] =
{
  { "friendly-cache", 'f', 0, G_OPTION_ARG_NONE, &opt_friendly_cache, "Store maps using friendly cache style (source name)", NULL },
  { "no-cache", 'n', 0, G_OPTION_ARG_NONE, &opt_no_cache, "Disable cache", NULL },
  { "cache-basedir", 'b', 0, G_OPTION_ARG_FILENAME, &opt_cache_base_dir, "Cache basedir", NULL },
  { "debug", 'd', 0, G_OPTION_ARG_NONE, &opt_debug, "Enable debugging", NULL },
  { "map", 'm', 0, G_OPTION_ARG_INT, &opt_map_provider, "Map source", "N" },
  { "editable-tracks", 'e', 0, G_OPTION_ARG_NONE, &opt_editable_tracks, "Make the tracks editable", NULL },
  { NULL }
};

static GdkPixbuf *g_star_image = NULL;
static OsmGpsMapImage *g_last_image = NULL;

static gboolean
on_button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
    OsmGpsMapPoint coord;
    float lat, lon;
    OsmGpsMap *map = OSM_GPS_MAP(widget);
    OsmGpsMapTrack *othertrack = OSM_GPS_MAP_TRACK(user_data);

    int left_button =   (event->button == 1) && (event->state == 0);
    int middle_button = (event->button == 2) || ((event->button == 1) && (event->state & GDK_SHIFT_MASK));
    int right_button =  (event->button == 3) || ((event->button == 1) && (event->state & GDK_CONTROL_MASK));

    osm_gps_map_convert_screen_to_geographic(map, event->x, event->y, &coord);
    osm_gps_map_point_get_degrees(&coord, &lat, &lon);

    if (event->type == GDK_3BUTTON_PRESS) {
        if (middle_button) {
            if (g_last_image)
                osm_gps_map_image_remove (map, g_last_image);
        }
        if (right_button) {
            osm_gps_map_track_remove(map, othertrack);
        }
    } else if (event->type == GDK_2BUTTON_PRESS) {
        if (left_button) {
            osm_gps_map_gps_add (map,
                                 lat,
                                 lon,
                                 g_random_double_range(0,360));
        }
        if (middle_button) {
            g_last_image = osm_gps_map_image_add (map,
                                                  lat,
                                                  lon,
                                                  g_star_image);
        }
        if (right_button) {
            osm_gps_map_track_add_point(othertrack, &coord);
        }
    }

    return FALSE;
}

static gboolean
on_button_release_event (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
    float lat,lon;
    GtkEntry *entry = GTK_ENTRY(user_data);
    OsmGpsMap *map = OSM_GPS_MAP(widget);

    g_object_get(map, "latitude", &lat, "longitude", &lon, NULL);
    gchar *msg = g_strdup_printf("Map Centre: lattitude %f longitude %f",lat,lon);
    gtk_entry_set_text(entry, msg);
    g_free(msg);

    return FALSE;
}

static gboolean
on_zoom_in_clicked_event (GtkWidget *widget, gpointer user_data)
{
    int zoom;
    OsmGpsMap *map = OSM_GPS_MAP(user_data);
    g_object_get(map, "zoom", &zoom, NULL);
    osm_gps_map_set_zoom(map, zoom+1);
    return FALSE;
}

static gboolean
on_zoom_out_clicked_event (GtkWidget *widget, gpointer user_data)
{
    int zoom;
    OsmGpsMap *map = OSM_GPS_MAP(user_data);
    g_object_get(map, "zoom", &zoom, NULL);
    osm_gps_map_set_zoom(map, zoom-1);
    return FALSE;
}

static gboolean
on_home_clicked_event (GtkWidget *widget, gpointer user_data)
{
    OsmGpsMap *map = OSM_GPS_MAP(user_data);
    osm_gps_map_set_center_and_zoom(map, -43.5326,172.6362,12);
    return FALSE;
}

static gboolean
on_cache_clicked_event (GtkWidget *widget, gpointer user_data)
{
    OsmGpsMap *map = OSM_GPS_MAP(user_data);
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) {
        int zoom,max_zoom;
        OsmGpsMapPoint pt1, pt2;
        osm_gps_map_get_bbox(map, &pt1, &pt2);
        g_object_get(map, "zoom", &zoom, "max-zoom", &max_zoom, NULL);
        osm_gps_map_download_maps(map, &pt1, &pt2, zoom, max_zoom);
    } else {
        osm_gps_map_download_cancel_all(map);
    }
    return FALSE;
}

static void
on_tiles_queued_changed (OsmGpsMap *image, GParamSpec *pspec, gpointer user_data)
{
    gchar *s;
    int tiles;
    GtkLabel *label = GTK_LABEL(user_data);
    g_object_get(image, "tiles-queued", &tiles, NULL);
    s = g_strdup_printf("%d", tiles);
    gtk_label_set_text(label, s);
    g_free(s);
}

static void
on_gps_alpha_changed (GtkAdjustment *adjustment, gpointer user_data)
{
    OsmGpsMap *map = OSM_GPS_MAP(user_data);
    OsmGpsMapTrack *track = osm_gps_map_gps_get_track (map);
    float f = gtk_adjustment_get_value(adjustment);
    g_object_set (track, "alpha", f, NULL);}

static void
on_gps_width_changed (GtkAdjustment *adjustment, gpointer user_data)
{
    OsmGpsMap *map = OSM_GPS_MAP(user_data);
    OsmGpsMapTrack *track = osm_gps_map_gps_get_track (map);
    float f = gtk_adjustment_get_value(adjustment);
    g_object_set (track, "line-width", f, NULL);
}

static void
on_star_align_changed (GtkAdjustment *adjustment, gpointer user_data)
{
    const char *propname = user_data;
    float f = gtk_adjustment_get_value(adjustment);
    if (g_last_image)
        g_object_set (g_last_image, propname, f, NULL);
}

#if GTK_CHECK_VERSION(3,4,0)
static void
on_gps_color_changed (GtkColorChooser *widget, gpointer user_data)
{
    GdkRGBA c;
    OsmGpsMapTrack *track = OSM_GPS_MAP_TRACK(user_data);
    gtk_color_chooser_get_rgba (widget, &c);
    osm_gps_map_track_set_color(track, &c);
}
#else
static void
on_gps_color_changed (GtkColorButton *widget, gpointer user_data)
{
    GdkRGBA c;
    OsmGpsMapTrack *track = OSM_GPS_MAP_TRACK(user_data);
    gtk_color_button_get_rgba (widget, &c);
    osm_gps_map_track_set_color(track, &c);
}

#endif

static void
on_close (GtkWidget *widget, gpointer user_data)
{
    gtk_widget_destroy(widget);
    gtk_main_quit();
}

static void
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
    GtkBuilder *builder;
    GtkWidget *widget;
    GtkAccelGroup *ag;
    OsmGpsMap *map;
    OsmGpsMapLayer *osd;
    OsmGpsMapTrack *rightclicktrack;
    const char *repo_uri;
    char *cachedir, *cachebasedir;
    GError *error = NULL;
    GOptionContext *context;

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

    cachebasedir = osm_gps_map_get_default_cache_directory();

    if (opt_cache_base_dir && g_file_test(opt_cache_base_dir, G_FILE_TEST_IS_DIR)) {
        cachedir = g_strdup(OSM_GPS_MAP_CACHE_AUTO);
        cachebasedir = g_strdup(opt_cache_base_dir);
    } else if (opt_friendly_cache) {
        cachedir = g_strdup(OSM_GPS_MAP_CACHE_FRIENDLY);
    } else if (opt_no_cache) {
        cachedir = g_strdup(OSM_GPS_MAP_CACHE_DISABLED);
    } else {
        cachedir = g_strdup(OSM_GPS_MAP_CACHE_AUTO);
    }

    if (opt_debug)
        gdk_window_set_debug_updates(TRUE);

    g_debug("Map Cache Dir: %s", cachedir);
    g_debug("Map Provider: %s (%d)", osm_gps_map_source_get_friendly_name(opt_map_provider), opt_map_provider);

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
    osm_gps_map_layer_add(OSM_GPS_MAP(map), osd);
    g_object_unref(G_OBJECT(osd));

    //Add a second track for right clicks
    rightclicktrack = osm_gps_map_track_new();

    if(opt_editable_tracks)
        g_object_set(rightclicktrack, "editable", TRUE, NULL);
    osm_gps_map_track_add(OSM_GPS_MAP(map), rightclicktrack);

    g_free(cachedir);
    g_free(cachebasedir);

    //Enable keyboard navigation
    osm_gps_map_set_keyboard_shortcut(map, OSM_GPS_MAP_KEY_FULLSCREEN, GDK_KEY_F11);
    osm_gps_map_set_keyboard_shortcut(map, OSM_GPS_MAP_KEY_UP, GDK_KEY_Up);
    osm_gps_map_set_keyboard_shortcut(map, OSM_GPS_MAP_KEY_DOWN, GDK_KEY_Down);
    osm_gps_map_set_keyboard_shortcut(map, OSM_GPS_MAP_KEY_LEFT, GDK_KEY_Left);
    osm_gps_map_set_keyboard_shortcut(map, OSM_GPS_MAP_KEY_RIGHT, GDK_KEY_Right);

    //Build the UI
    g_star_image = gdk_pixbuf_new_from_file_at_size ("poi.png", 24,24,NULL);

    builder = gtk_builder_new();
    gtk_builder_add_from_file (builder, "mapviewer.ui", &error);
    if (error)
        g_error ("ERROR: %s\n", error->message);

    gtk_box_pack_start (
                GTK_BOX(gtk_builder_get_object(builder, "map_box")),
                GTK_WIDGET(map), TRUE, TRUE, 0);

    //Init values
    float lw,a;
    GdkRGBA c;
    OsmGpsMapTrack *gpstrack = osm_gps_map_gps_get_track (map);
    g_object_get (gpstrack, "line-width", &lw, "alpha", &a, NULL);
    osm_gps_map_track_get_color(gpstrack, &c);
    gtk_adjustment_set_value (
                GTK_ADJUSTMENT(gtk_builder_get_object(builder, "gps_width_adjustment")),
                lw);
    gtk_adjustment_set_value (
                GTK_ADJUSTMENT(gtk_builder_get_object(builder, "gps_alpha_adjustment")),
                a);
    gtk_adjustment_set_value (
                GTK_ADJUSTMENT(gtk_builder_get_object(builder, "star_xalign_adjustment")),
                0.5);
    gtk_adjustment_set_value (
                GTK_ADJUSTMENT(gtk_builder_get_object(builder, "star_yalign_adjustment")),
                0.5);
#if GTK_CHECK_VERSION(3,4,0)
    gtk_color_chooser_set_rgba (
                GTK_COLOR_CHOOSER(gtk_builder_get_object(builder, "gps_colorbutton")),
                &c);
#else
    gtk_color_button_set_rgba (
                GTK_COLOR_BUTTON(gtk_builder_get_object(builder, "gps_colorbutton")),
                &c);
#endif

    //Connect to signals
    g_signal_connect (
                gtk_builder_get_object(builder, "gps_colorbutton"), "color-set",
                G_CALLBACK (on_gps_color_changed), (gpointer) gpstrack);
    g_signal_connect (
                gtk_builder_get_object(builder, "zoom_in_button"), "clicked",
                G_CALLBACK (on_zoom_in_clicked_event), (gpointer) map);
    g_signal_connect (
                gtk_builder_get_object(builder, "zoom_out_button"), "clicked",
                G_CALLBACK (on_zoom_out_clicked_event), (gpointer) map);
    g_signal_connect (
                gtk_builder_get_object(builder, "home_button"), "clicked",
                G_CALLBACK (on_home_clicked_event), (gpointer) map);
    g_signal_connect (
                gtk_builder_get_object(builder, "cache_button"), "clicked",
                G_CALLBACK (on_cache_clicked_event), (gpointer) map);
    g_signal_connect (
                gtk_builder_get_object(builder, "gps_alpha_adjustment"), "value-changed",
                G_CALLBACK (on_gps_alpha_changed), (gpointer) map);
    g_signal_connect (
                gtk_builder_get_object(builder, "gps_width_adjustment"), "value-changed",
                G_CALLBACK (on_gps_width_changed), (gpointer) map);
    g_signal_connect (
                gtk_builder_get_object(builder, "star_xalign_adjustment"), "value-changed",
                G_CALLBACK (on_star_align_changed), (gpointer) "x-align");
    g_signal_connect (
                gtk_builder_get_object(builder, "star_yalign_adjustment"), "value-changed",
                G_CALLBACK (on_star_align_changed), (gpointer) "y-align");
    g_signal_connect (G_OBJECT (map), "button-press-event",
                G_CALLBACK (on_button_press_event), (gpointer) rightclicktrack);
    g_signal_connect (G_OBJECT (map), "button-release-event",
                G_CALLBACK (on_button_release_event),
                (gpointer) gtk_builder_get_object(builder, "text_entry"));
    g_signal_connect (G_OBJECT (map), "notify::tiles-queued",
                G_CALLBACK (on_tiles_queued_changed),
                (gpointer) gtk_builder_get_object(builder, "cache_label"));

    widget = GTK_WIDGET(gtk_builder_get_object(builder, "window1"));
    g_signal_connect (widget, "destroy",
                      G_CALLBACK (on_close), (gpointer) map);
    //Setup accelerators.
    ag = gtk_accel_group_new();
    gtk_accel_group_connect(ag, GDK_KEY_w, GDK_CONTROL_MASK, GTK_ACCEL_MASK,
                    g_cclosure_new(gtk_main_quit, NULL, NULL));
    gtk_accel_group_connect(ag, GDK_KEY_q, GDK_CONTROL_MASK, GTK_ACCEL_MASK,
                    g_cclosure_new(gtk_main_quit, NULL, NULL));
    gtk_window_add_accel_group(GTK_WINDOW(widget), ag);

    gtk_widget_show_all (widget);

    g_log_set_handler ("OsmGpsMap", G_LOG_LEVEL_MASK, g_log_default_handler, NULL);
    gtk_main ();

    return 0;
}
