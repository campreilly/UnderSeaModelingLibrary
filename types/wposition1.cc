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
wposition1::wposition1(double lat, double lng, double alt) {
    latitude(lat);
    longitude(lng);
    altitude(alt);
}

/**
 * Constructs a position on the Earth's surface using the great circle
 * range and bearing from a point.
 */
wposition1::wposition1( const wposition1& location, double range, double bearing) {
    const double lat1 = to_radians( location.latitude() ) ;
    const double lng1 = to_radians( location.longitude() ) ;
    const double R = wposition::earth_radius + location.altitude() ;
    range /= R ;    // converts to radians

    double lat2 = asin( sin(lat1)*cos(range) + cos(lat1)*sin(range)*cos(bearing) ) ;
    double lng2 = lng1 + atan2(
        sin(bearing) * sin(range) * cos(lat1),
        cos(range) - sin(lat1) * sin(lat2) ) ;
    latitude( to_degrees(lat2) );
    longitude( to_degrees(lng2) );
    altitude( location.altitude() );
}

/**
 * Constructs a new position as a copy of an existing position.
 */
wposition1::wposition1(const wvector1& other) :
        wvector1(other) {
}

/**
 * Copy one element from a matrix of world vectors.
 */
wposition1::wposition1(const wvector& other, unsigned row, unsigned col) {
    rho(other.rho(row, col));
    theta(other.theta(row, col));
    phi(other.phi(row, col));
}

/**
 * Computes the great circle range and bearing between two
 * latitude/longtiude positions along the Earth's surface.
 */
double wposition1::gc_range( const wposition1& location, double* bearing ) const {
    const double lat1 = to_radians( latitude() ) ;
    const double lng1 = to_radians( longitude() ) ;
    const double R = wposition::earth_radius + altitude() ;
    const double lat2 = to_radians( location.latitude() ) ;
    const double lng2 = to_radians( location.longitude() ) ;

    // compute range (in radians) using the Haversine formula

    double hav1at = sin( 0.5*(lat1-lat2) ) ; hav1at *= hav1at ;
    double havlng = sin( 0.5*(lng1-lng2) ) ; havlng *= havlng ;
    double range = 2.0 * asin( sqrt( hav1at + cos(lat1)*cos(lat2)*havlng ) ) ;

    // compute bearing

    if ( bearing ) {
        if ( cos(lat1) < 1e-10 ) {
            if ( lat1 > 0 ) {       // starting from north pole
                *bearing = M_PI ;
            } else {                // starting from south pole
                *bearing = 0.0 ;
            }
        } else {                    // starting anywhere else
            *bearing = acos( (sin(lat2)-sin(lat1)*cos(range))
                     / (sin(range)*cos(lat1)) ) ;
            if ( sin(lng2-lng1) < 0 ) {
                *bearing = TWO_PI - *bearing ;
            }
        }
    }

    // return range in meters

    return range * R ;
}
