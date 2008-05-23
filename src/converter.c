/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * converter.c
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

#include <math.h>
#include <stdio.h>

#include "osm-gps-map-types.h"
#include "converter.h"


float
deg2rad(float deg)
{
	return (deg * M_PI / 180);
}

float
rad2deg(float rad)
{
	return (rad / M_PI * 180);
}


int
lat2pixel(	float zoom,
		float lat)
{
	float lat_m;
	int pixel_y;

	lat_m = atanh(sin(lat));
	
	// printf("lat %f => lat_m %f \n", lat,lat_m);
	
	pixel_y = -( (lat_m * TILESIZE * exp(zoom * M_LN2) ) / (2*M_PI)) + 
		    (exp(zoom * M_LN2) * (TILESIZE/2) );


	return pixel_y;
}


int
lon2pixel(	float zoom,
		float lon)
{
	int pixel_x;

	pixel_x = ( (lon * TILESIZE * exp(zoom * M_LN2) ) / (2*M_PI) ) + 
		    ( exp(zoom * M_LN2) * (TILESIZE/2) );
	return pixel_x;
}

float
pixel2lon(	float zoom,
		int pixel_x)
{
	float lon;
	
	lon = ((pixel_x - ( exp(zoom * M_LN2) * (TILESIZE/2) ) ) *2*M_PI) / 
			(TILESIZE * exp(zoom * M_LN2) );
	
	return lon;
}

float
pixel2lat(	float zoom,
		int pixel_y)
{
	float lat, lat_m;

	lat_m = (-( pixel_y - ( exp(zoom * M_LN2) * (TILESIZE/2) ) ) * (2*M_PI)) /
				(TILESIZE * exp(zoom * M_LN2));
	
	lat = asin(tanh(lat_m));
	
	printf("lat %f => lat_m %f \n", lat,lat_m);
	
	
	return lat;
}
