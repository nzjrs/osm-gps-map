float
deg2rad(float deg);

float
rad2deg(float rad);

int
lat2pixel(	float zoom,
		float lat);

int
lon2pixel(	float zoom,
		float lon);
		
float
pixel2lon(	float zoom,
		int pixel_x);
		
float
pixel2lat(	float zoom,
		int pixel_y);
