/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * main.c
 * Copyright (C) John Stowers 2008 <john.stowers@gmail.com>
 * 
 * main.c is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * main.c is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib.h>
#include <gtk/gtk.h>
#include "osm-gps-map.h"

#define USE_GOOGLE 0

static GtkWidget *map;

gboolean
on_button_press_event (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	g_debug("PRESS");
	return FALSE;
}

gboolean
on_button_release_event (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	g_debug("RELEASE");
	return FALSE;
}

gboolean 
on_zoom_in_clicked_event (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	g_debug("+");
}

gboolean 
on_zoom_out_clicked_event (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	g_debug("-");
}

gboolean 
on_home_clicked_event (GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{
	osm_gps_map_set_mapcenter(map, -43.5326,172.6362,12);
	g_debug("H");
}



int
main (int argc, char **argv)
{
	GtkWidget *vbox;
	GtkWidget *bbox;
	GtkWidget *window;
	GtkWidget *zoomInbutton;
	GtkWidget *zoomOutbutton;
	GtkWidget *homeButton;

	g_thread_init(NULL);
	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
	
#if USE_GOOGLE	
	//According to 
	//http://www.mgmaps.com/cache/MapTileCacher.perl
	//the v string means:
	//  w2.99		Maps
	//  w2t.99		Hybrid
	//  w2p.99		Photo
	map = g_object_new (OSM_TYPE_GPS_MAP,
						"repo-uri","http://mt.google.com/mt?n=404&v=w2p.99&x=%d&y=%d&zoom=%d",
						"tile-cache","/tmp/Maps/Google",
						"invert-zoom",TRUE,
						NULL);
#else
	map = osm_gps_map_new ();
#endif
  	g_signal_connect (map, "button-press-event",
    		G_CALLBACK (on_button_press_event),NULL);
	g_signal_connect (map, "button_release_event",
            G_CALLBACK (on_button_release_event),NULL);



    vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (window), vbox);

	//Add the map to the box
	gtk_box_pack_start (GTK_BOX(vbox), map, TRUE, TRUE, 0);
	//And add a hbox
    bbox = gtk_hbox_new (TRUE, 0);
	gtk_box_pack_start (GTK_BOX(vbox), bbox, FALSE, TRUE, 0);

	//Add buttons to the bbox
	zoomInbutton = gtk_button_new_with_label ("+");
    g_signal_connect (G_OBJECT (zoomInbutton), "clicked",
		      G_CALLBACK (on_zoom_in_clicked_event), (gpointer) map);
	gtk_box_pack_start (GTK_BOX(bbox), zoomInbutton, FALSE, TRUE, 0);

	zoomOutbutton= gtk_button_new_with_label ("-");
    g_signal_connect (G_OBJECT (zoomOutbutton), "clicked",
		      G_CALLBACK (on_zoom_out_clicked_event), (gpointer) map);
	gtk_box_pack_start (GTK_BOX(bbox), zoomOutbutton, FALSE, TRUE, 0);

	homeButton = gtk_button_new_with_label ("Home");
    g_signal_connect (G_OBJECT (homeButton), "clicked",
		      G_CALLBACK (on_home_clicked_event), (gpointer) map);
	gtk_box_pack_start (GTK_BOX(bbox), homeButton, FALSE, TRUE, 0);


	g_signal_connect (window, "destroy",
			G_CALLBACK (gtk_main_quit), NULL);

	gtk_widget_show_all (window);

	gtk_main ();
	return 0;
}
