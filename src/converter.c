/***************************************************************************
 *  Copyright  2008  Marcus Bauer
 *  License    GPLv2
 ****************************************************************************/

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
