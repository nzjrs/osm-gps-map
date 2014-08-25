/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*- */
/* vim:set et sw=4 ts=4 */
/*
 * Copyright (C) Marcus Bauer 2008 <marcus.bauer@gmail.com>
 * Copyright (C) 2013 John Stowers <john.stowers@gmail.com>
 * Copyright (C) 2014 Martijn Goedhart <goedhart.martijn@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <math.h>
#include <stdio.h>
#include <float.h>

#include "private.h"
#include "converter.h"

/*
 * The (i)logb(x) function is equal to `floor(log(x) / log(2))` or
 * `floor(log2(x))`, but probably faster and also accepts negative values.
 * But this is only true when FLT_RADIX equals 2, which is on allmost all
 * machine.
 */
#if FLT_RADIX == 2
#define LOG2(x) (ilogb(x))
#else
#define LOG2(x) ((int)floor(log2(abs(x))))
#endif

float
deg2rad(float deg)
{
    return (deg * M_PI / 180.0);
}

float
rad2deg(float rad)
{
    return (rad / M_PI * 180.0);
}


int
lat2pixel(  int zoom,
            float lat)
{
    float lat_m;
    int pixel_y;

    lat_m = atanh(sin(lat));

    /* the formula is
     *
     * some more notes
     * http://manialabs.wordpress.com/2013/01/26/converting-latitude-and-longitude-to-map-tile-in-mercator-projection/
     *
     * pixel_y = -(2^zoom * TILESIZE * lat_m) / 2PI + (2^zoom * TILESIZE) / 2
     */
    pixel_y = -(int)( (lat_m * TILESIZE * (1 << zoom) ) / (2*M_PI)) +
        ((1 << zoom) * (TILESIZE/2) );


    return pixel_y;
}


int
lon2pixel(  int zoom,
            float lon)
{
    int pixel_x;

    /* the formula is
     *
     * pixel_x = (2^zoom * TILESIZE * lon) / 2PI + (2^zoom * TILESIZE) / 2
     */
    pixel_x = (int)(( lon * TILESIZE * (1 << zoom) ) / (2*M_PI)) +
        ( (1 << zoom) * (TILESIZE/2) );
    return pixel_x;
}

float
pixel2lon(  float zoom,
            int pixel_x)
{
    float lon;

    lon = ((pixel_x - ( exp(zoom * M_LN2) * (TILESIZE/2) ) ) *2*M_PI) / 
        (TILESIZE * exp(zoom * M_LN2) );

    return lon;
}

float
pixel2lat(  float zoom,
            int pixel_y)
{
    float lat, lat_m;

    lat_m = (-( pixel_y - ( exp(zoom * M_LN2) * (TILESIZE/2) ) ) * (2*M_PI)) /
        (TILESIZE * exp(zoom * M_LN2));

    lat = asin(tanh(lat_m));

    return lat;
}

int
latlon2zoom(int pix_height,
	    int pix_width,
	    float lat1,
	    float lat2,
	    float lon1,
	    float lon2)
{
    float lat1_m = atanh(sin(lat1));
    float lat2_m = atanh(sin(lat2));
    int zoom_lon = LOG2((double)(2 * pix_width * M_PI) / (TILESIZE * (lon2 - lon1)));
    int zoom_lat = LOG2((double)(2 * pix_height * M_PI) / (TILESIZE * (lat2_m - lat1_m)));
    return MIN(zoom_lon, zoom_lat);
}
