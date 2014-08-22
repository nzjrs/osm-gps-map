#include <gtk/gtk.h>
#include "osm-gps-map.h"

int
main (int   argc,
      char *argv[])
{
	OsmGpsMap *map;
	OsmGpsMapPolygon* poly;
    GtkWidget *window;

    gtk_init (&argc, &argv);

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window), "Window");
    g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);


	map = g_object_new (OSM_TYPE_GPS_MAP, NULL);
	gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(map));

	poly = osm_gps_map_polygon_new();
	OsmGpsMapTrack* polytrack = osm_gps_map_track_new();

	OsmGpsMapPoint* p1, *p2, *p3;
	p1 = osm_gps_map_point_new_radians(1.25663706, -0.488692191);
	p2 = osm_gps_map_point_new_radians(1.06465084, -0.750491578);
	p3 = osm_gps_map_point_new_radians(1.064650849, -0.191986218);

	osm_gps_map_track_add_point(polytrack, p1);
	osm_gps_map_track_add_point(polytrack, p2);
	osm_gps_map_track_add_point(polytrack, p3);

	g_object_set(poly, "track", polytrack, NULL);
	g_object_set(poly, "editable", TRUE, NULL);

	osm_gps_map_polygon_add(map, poly);

	gtk_widget_show (GTK_WIDGET(map));
    gtk_widget_show (window);

    gtk_main ();

    return 0;
}


