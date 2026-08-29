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
#include <cairo.h>

#include "osm-gps-map.h"

static OsmGpsMapSource_t opt_map_provider = OSM_GPS_MAP_SOURCE_OPENSTREETMAP;
static gboolean opt_friendly_cache = FALSE;
static gboolean opt_no_cache = FALSE;
static char *opt_cache_base_dir = NULL;
static gboolean opt_editable_tracks = FALSE;
static GOptionEntry entries[] =
{
  { "friendly-cache", 'f', 0, G_OPTION_ARG_NONE, &opt_friendly_cache, "Store maps using friendly cache style (source name)", NULL },
  { "no-cache", 'n', 0, G_OPTION_ARG_NONE, &opt_no_cache, "Disable cache", NULL },
  { "cache-basedir", 'b', 0, G_OPTION_ARG_FILENAME, &opt_cache_base_dir, "Cache basedir", NULL },
  { "map", 'm', 0, G_OPTION_ARG_INT, &opt_map_provider, "Map source", "N" },
  { "editable-tracks", 'e', 0, G_OPTION_ARG_NONE, &opt_editable_tracks, "Make the tracks editable", NULL },
  { NULL }
};

#if !GTK_CHECK_VERSION(3,22,0)
// use --gtk-debug=updates instead on newer GTK
static gboolean opt_debug = FALSE;
static GOptionEntry debug_entries[] =
{
  { "debug", 'd', 0, G_OPTION_ARG_NONE, &opt_debug, "Enable debugging", NULL },
  { NULL }
};
#endif

/* Shared with mapviewer.py DrawLayer Home (Timaru). */
#define HOME_LAT  -44.39
#define HOME_LON  171.25
#define HOME_ZOOM 12

typedef struct _DrawLayer {
    GObject parent;
} DrawLayer;

typedef struct _DrawLayerClass {
    GObjectClass parent_class;
} DrawLayerClass;

static void draw_layer_interface_init (OsmGpsMapLayerIface *iface);

G_DEFINE_TYPE_WITH_CODE (DrawLayer, draw_layer, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (OSM_TYPE_GPS_MAP_LAYER,
                                                draw_layer_interface_init));

static void
draw_layer_draw (OsmGpsMapLayer *layer G_GNUC_UNUSED,
                 OsmGpsMap *map, cairo_t *cr)
{
    OsmGpsMapPoint *pt;
    gint x, y;

    pt = osm_gps_map_point_new_degrees (HOME_LAT, HOME_LON);
    osm_gps_map_convert_geographic_to_screen (map, pt, &x, &y);
    osm_gps_map_point_free (pt);

    cairo_set_source_rgba (cr, 1.0, 0.0, 0.0, 0.6);
    cairo_move_to (cr, x, y);
    cairo_line_to (cr, x + 12, y + 10);
    cairo_line_to (cr, x + 12, y + 24);
    cairo_line_to (cr, x - 12, y + 24);
    cairo_line_to (cr, x - 12, y + 10);
    cairo_close_path (cr);
    cairo_fill (cr);
}

static void
draw_layer_render (OsmGpsMapLayer *layer G_GNUC_UNUSED,
                   OsmGpsMap *map G_GNUC_UNUSED)
{
}

static gboolean
draw_layer_busy (OsmGpsMapLayer *layer G_GNUC_UNUSED)
{
    return FALSE;
}

static gboolean
draw_layer_button_press (OsmGpsMapLayer *layer G_GNUC_UNUSED,
                         OsmGpsMap *map G_GNUC_UNUSED,
                         GdkEventButton *event G_GNUC_UNUSED)
{
    return FALSE;
}

static void
draw_layer_interface_init (OsmGpsMapLayerIface *iface)
{
    iface->render = draw_layer_render;
    iface->draw = draw_layer_draw;
    iface->busy = draw_layer_busy;
    iface->button_press = draw_layer_button_press;
}

static void
draw_layer_class_init (DrawLayerClass *klass G_GNUC_UNUSED)
{
}

static void
draw_layer_init (DrawLayer *self G_GNUC_UNUSED)
{
}

static GdkPixbuf *g_star_image = NULL;
static OsmGpsMapImage *g_last_image = NULL;
static OsmGpsMapTrack *g_click_track = NULL;

/* Programmatic marker if poi.png is missing. Live cairo overlay is DrawLayer. */
static GdkPixbuf *
star_pixbuf_new (int size)
{
    cairo_surface_t *surface;
    cairo_t *cr;
    GdkPixbuf *pixbuf;
    int i;
    double cx, cy, outer, inner;

    surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, size, size);
    cr = cairo_create (surface);
    cx = cy = size / 2.0;
    outer = size / 2.0 - 1.0;
    inner = size / 5.0;
    for (i = 0; i < 10; i++) {
        double r = (i % 2 == 0) ? outer : inner;
        double ang = -G_PI / 2.0 + i * G_PI / 5.0;
        double x = cx + r * cos (ang);
        double y = cy + r * sin (ang);
        if (i == 0)
            cairo_move_to (cr, x, y);
        else
            cairo_line_to (cr, x, y);
    }
    cairo_close_path (cr);
    cairo_set_source_rgb (cr, 1.0, 0.85, 0.0);
    cairo_fill_preserve (cr);
    cairo_set_source_rgb (cr, 0.75, 0.45, 0.0);
    cairo_set_line_width (cr, 1);
    cairo_stroke (cr);
    pixbuf = gdk_pixbuf_get_from_surface (surface, 0, 0, size, size);
    cairo_destroy (cr);
    cairo_surface_destroy (surface);
    return pixbuf;
}

static OsmGpsMapTrack *
click_track_new (void)
{
    OsmGpsMapTrack *track = osm_gps_map_track_new ();
    if (opt_editable_tracks)
        g_object_set (track, "editable", TRUE, NULL);
    return track;
}

static gboolean
on_button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer user_data G_GNUC_UNUSED)
{
    OsmGpsMapPoint coord;
    float lat, lon;
    OsmGpsMap *map = OSM_GPS_MAP(widget);
    /* 2BUTTON_PRESS often has BUTTONn_MASK set; ignore those, keep Shift/Ctrl. */
    GdkModifierType mods = event->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK | GDK_MOD1_MASK);

    int left_button =   (event->button == 1) && (mods == 0);
    int middle_button = (event->button == 2) || ((event->button == 1) && (event->state & GDK_SHIFT_MASK));
    int right_button =  (event->button == 3) || ((event->button == 1) && (event->state & GDK_CONTROL_MASK));

    osm_gps_map_convert_screen_to_geographic(map, event->x, event->y, &coord);
    osm_gps_map_point_get_degrees(&coord, &lat, &lon);

    if (event->type == GDK_3BUTTON_PRESS) {
        if (middle_button) {
            if (g_last_image) {
                osm_gps_map_image_remove (map, g_last_image);
                g_last_image = NULL;
            }
        }
        if (right_button) {
            osm_gps_map_track_remove (map, g_click_track);
            g_object_unref (g_click_track);
            g_click_track = click_track_new ();
            osm_gps_map_track_add (map, g_click_track);
        }
    } else if (event->type == GDK_2BUTTON_PRESS) {
        if (left_button) {
            osm_gps_map_gps_add (map,
                                 lat,
                                 lon,
                                 g_random_double_range(0,360));
        }
        if (middle_button && g_star_image) {
            g_last_image = osm_gps_map_image_add (map,
                                                  lat,
                                                  lon,
                                                  g_star_image);
        }
        if (right_button) {
            osm_gps_map_track_add_point(g_click_track, &coord);
        }
    }

    return FALSE;
}

static gboolean
on_map_changed_event (GtkWidget *widget, gpointer user_data)
{
    float lat,lon;
    GtkEntry *entry = GTK_ENTRY(user_data);
    OsmGpsMap *map = OSM_GPS_MAP(widget);

    g_object_get(map, "latitude", &lat, "longitude", &lon, NULL);
    gchar *msg = g_strdup_printf("Map Centre: latitude %f longitude %f",lat,lon);
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
    osm_gps_map_set_center_and_zoom(map, HOME_LAT, HOME_LON, HOME_ZOOM);
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
    OsmGpsMapLayer *osd, *draw;
    const char *repo_uri;
    char *cachedir, *cachebasedir;
    GError *error = NULL;
    GOptionContext *context;

    gtk_init (&argc, &argv);

    context = g_option_context_new ("- Map browser");
    g_option_context_set_help_enabled(context, FALSE);
    g_option_context_add_main_entries (context, entries, NULL);
#if !GTK_CHECK_VERSION(3,22,0)
    g_option_context_add_main_entries (context, debug_entries, NULL);
#endif

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

#if !GTK_CHECK_VERSION(3,22,0)
// use --gtk-debug=updates on newer gtk
    if (opt_debug)
        gdk_window_set_debug_updates(TRUE);
#endif

    g_debug("Map Cache Dir: %s", cachedir);
    g_debug("Map Provider: %s (%d)", osm_gps_map_source_get_friendly_name(opt_map_provider), opt_map_provider);

    map = g_object_new (OSM_TYPE_GPS_MAP,
                        "map-source",opt_map_provider,
                        "tile-cache",cachedir,
                        "tile-cache-base", cachebasedir,
                        "proxy-uri",g_getenv("http_proxy"),
                        "user-agent", "mapviewer.c", // Always set user-agent, for better tile-usage compliance
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

    draw = g_object_new (draw_layer_get_type (), NULL);
    osm_gps_map_layer_add (OSM_GPS_MAP (map), draw);
    g_object_unref (draw);

    /* Stay put on gps_add so the blue blob is not under the OSD crosshair. */
    g_object_set (map, "auto-center", FALSE, NULL);

    g_click_track = click_track_new ();
    osm_gps_map_track_add (OSM_GPS_MAP (map), g_click_track);
    osm_gps_map_set_center_and_zoom (map, HOME_LAT, HOME_LON, HOME_ZOOM);

    g_free(cachedir);
    g_free(cachebasedir);

    //Enable keyboard navigation
    osm_gps_map_set_keyboard_shortcut(map, OSM_GPS_MAP_KEY_FULLSCREEN, GDK_KEY_F11);
    osm_gps_map_set_keyboard_shortcut(map, OSM_GPS_MAP_KEY_UP, GDK_KEY_Up);
    osm_gps_map_set_keyboard_shortcut(map, OSM_GPS_MAP_KEY_DOWN, GDK_KEY_Down);
    osm_gps_map_set_keyboard_shortcut(map, OSM_GPS_MAP_KEY_LEFT, GDK_KEY_Left);
    osm_gps_map_set_keyboard_shortcut(map, OSM_GPS_MAP_KEY_RIGHT, GDK_KEY_Right);

    //Build the UI
    {
        GError *pixbuf_err = NULL;
        g_star_image = gdk_pixbuf_new_from_file_at_size ("poi.png", 24, 24, &pixbuf_err);
        if (!g_star_image) {
            g_printerr ("poi.png: %s\n", pixbuf_err ? pixbuf_err->message : "missing");
            g_clear_error (&pixbuf_err);
            g_star_image = star_pixbuf_new (24);
        }
    }

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
                G_CALLBACK (on_button_press_event), NULL);
    g_signal_connect (G_OBJECT (map), "changed",
                G_CALLBACK (on_map_changed_event),
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
