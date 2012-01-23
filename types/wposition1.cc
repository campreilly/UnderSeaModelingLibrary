/**
 * @file wposition1.cc
 * Individual world location in geodetic earth coordinates 
 * (latitude, longitude, and altitude).
 */
#include <usml/types/wposition1.h>

using namespace usml::types;

/** 
 * Constructs an individual position.
 */
wposition1::wposition1(double lat, double lng, double alt)
{
    latitude(lat);
    longitude(lng);
    altitude(alt);
}

/** 
 * Constructs a new position as a copy of an existing position.
 */
wposition1::wposition1(const wvector1& other) :
    wvector1(other)
{
}

/**
 * Copy one element from a matrix of world vectors.
 */
wposition1::wposition1(const wvector& other, unsigned row, unsigned col)
{
    rho(other.rho(row, col));
    theta(other.theta(row, col));
    phi(other.phi(row, col));
}
