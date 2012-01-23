/**
 * @file wposition.cc
 * World location in geodetic earth coordinates 
 * (latitude, longitude, and altitude). 
 */
#include <usml/types/wposition1.h>

using namespace usml::types;

/** 
 * Constructs a matrix of wpositions. Initializes each wposition to
 * the center of the earth.
 */
wposition::wposition(unsigned rows, unsigned cols, double latitude,
        double longitude, double altitude) :
    wvector(rows, cols)
{
    scalar_matrix<double> lat(rows, cols, latitude);
    scalar_matrix<double> lng(rows, cols, longitude);
    scalar_matrix<double> alt(rows, cols, altitude);
    this->latitude(lat);
    this->longitude(lng);
    this->altitude(alt);
}

/**
 * Local radius of curvature in the area of operations.
 */
double wposition::earth_radius = 6378101.030201019;

/**
 * Compute the radius of the prime vertical using the WGS-84 formula.
 */
void wposition::compute_earth_radius(double latitude)
{
    const double a = 6378137.0;
    const double f = 1.0 / 298.257223563;
    const double e2 = f * (2.0 - f);
    const double sinT = sin(to_radians(latitude));
    const double w = sqrt(1.0 - e2 * sinT * sinT);
    const double rm = a * (1-e2) / (w*w*w) ;
    const double rv = a / w ;
    earth_radius = sqrt( rm * rv );
}
