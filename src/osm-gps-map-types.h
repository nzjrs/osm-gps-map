/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * osm-gps-map-types.h
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

#ifndef _OSM_GPS_MAP_TYPES_H_
#define _OSM_GPS_MAP_TYPES_H_

#include <gdk/gdk.h>
#include "osm-gps-map.h"

#define TILE_POINT_SIZE 5
#define TILE_LINE_SIZE  4

#define TILESIZE 256
#define MAX_ZOOM 20
#define MIN_ZOOM 0

#define URI_MARKER_X 	"#X"
#define URI_MARKER_Y 	"#Y"
#define URI_MARKER_Z 	"#Z"
#define URI_MARKER_S 	"#S"
#define URI_MARKER_Q 	"#Q"
#define URI_MARKER_Q0 	"#W"
#define URI_MARKER_YS 	"#U"
#define URI_MARKER_R 	"#R"

#define URI_HAS_X 	(1 << 0)
#define URI_HAS_Y 	(1 << 1)
#define URI_HAS_Z 	(1 << 2)
#define URI_HAS_S 	(1 << 3)
#define URI_HAS_Q 	(1 << 4)
#define URI_HAS_Q0 	(1 << 5)
#define URI_HAS_YS 	(1 << 6)
#define URI_HAS_R 	(1 << 7)
//....
#define URI_FLAG_END (1 << 8)

typedef struct {
	int x1;
	int y1;
	int x2;
	int y2;
} bbox_pixel_t;

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

#endif /* _OSM_GPS_MAP_TYPES_H_ */
