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


#include <gtk/gtk.h>
#include "osm-gps-map.h"

#define USE_GOOGLE 0

int
main (int argc, char **argv)
{
	GtkWidget *window;
	GtkWidget *map;

	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	
#if USE_GOOGLE	
	//According to 
	//http://www.mgmaps.com/cache/MapTileCacher.perl
	//the v string means:
	//  w2.99		Maps
	//  w2t.99		Hybrid
	//  w2p.99		Photo
	map = g_object_new (OSM_TYPE_GPS_MAP,
						"repo-uri","http://mt.google.com/mt?n=404&v=w2.99&x=%d&y=%d&zoom=%d",
						"tile-cache","/tmp/Maps/Google",
						"invert-zoom",TRUE,
						NULL);
#else
	map = osm_gps_map_new ();
#endif
	
	gtk_container_add (GTK_CONTAINER (window), map);

	g_signal_connect (window, "destroy",
			G_CALLBACK (gtk_main_quit), NULL);

	gtk_widget_show_all (window);

	gtk_main ();
	return 0;
}
