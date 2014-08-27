/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*- */
/* vim:set et sw=4 ts=4 cino=t0,(0: */
/*
 * clickable_track.c
 * Copyright (C) Martijn Goedhart 2014 <goedhart.martijn@gmail.com>
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

#include <gtk/gtk.h>
#include "osm-gps-map.h"
#include "converter.h"

void point_clicked(OsmGpsMapTrack *osmgpsmaptrack, OsmGpsMapPoint *point, gpointer user_data);

void
point_clicked(OsmGpsMapTrack *osmgpsmaptrack, OsmGpsMapPoint *point, gpointer user_data)
{
    printf("point at latitude: %.4f and longitude %.4f clicked\n", rad2deg(point->rlat), rad2deg(point->rlon));
}

int
main (int argc, char *argv[])
{
    OsmGpsMap *map;
    GtkWidget *window;

    gtk_init(&argc, &argv);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Window");
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    map = g_object_new(OSM_TYPE_GPS_MAP, NULL);
    gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(map));

    OsmGpsMapTrack* track = osm_gps_map_track_new();

    OsmGpsMapPoint* p1, *p2, *p3, *p4;
    p1 = osm_gps_map_point_new_radians(1.25663706, -0.488692191);
    p2 = osm_gps_map_point_new_radians(1.06465084, -0.750491578);
    p3 = osm_gps_map_point_new_radians(1.17245321, -0.685401453);
    p4 = osm_gps_map_point_new_radians(1.04543154, -0.105454354);

    osm_gps_map_track_add_point(track, p1);
    osm_gps_map_track_add_point(track, p2);
    osm_gps_map_track_add_point(track, p3);
    osm_gps_map_track_add_point(track, p4);

    osm_gps_map_point_free(p1);
    osm_gps_map_point_free(p2);
    osm_gps_map_point_free(p3);
    osm_gps_map_point_free(p4);

    g_object_set(track, "clickable", TRUE, NULL);
    g_signal_connect(track, "point-clicked", G_CALLBACK(point_clicked), NULL);

    osm_gps_map_track_add(map, track);

    gtk_widget_show(GTK_WIDGET(map));
    gtk_widget_show(window);

    gtk_main();

    return 0;
}
