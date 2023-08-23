/**
 * @file wposition1.h
 * Individual world location in geodetic earth coordinates (latitude, longitude, and altitude).
 */
#pragma once

#include <usml/ublas/ublas.h>
#include <usml/types/wvector1.h>
#include <usml/types/wposition.h>

namespace usml {
namespace types {
/// @ingroup wposition
/// @{

/**
 * World location in geodetic earth coordinates (latitude, longitude,
 * and altitude). Each of the three coordinate parameters is a scalar.
 * This class is a convenience to save the developer from doing a lot of
 * operations on 1x1 matrices.
 */
class USML_DECLSPEC wposition1: public wvector1
{

public:

    //*********************************
    // initialization

    /**
     * Constructs an individual position.
     *
     * @param  lat          Initial latitude (degrees).
     * @param  lng          Initial longitude (degrees).
     * @param  alt          Initial altitude (meters).
     */
    wposition1(double lat = 0.0, double lng = 0.0, double alt = 0.0);

    /**
     * Constructs a position on the Earth's surface using the great circle
     * range and bearing from a point.
     *
     * @xref E. Williams, "Aviation Formulary V1.43",
     * http://williams.best.vwh.net/avform.htm , July 2010.
     *
     * @param  location     Location of the reference point.
     * @param  range        Distance to the new point along the Earth's surface (meters).
     * @param  bearing      True bearing to the new point (radians).
     */
    wposition1( const wposition1& location, double range, double bearing);

    /**
     * Constructs a new position as a copy of an existing position.
     * Accepts either an actual wposition, or one of its superclasses.
     *
     * @param  other        wposition1 to be copied.
     */
    wposition1(const wvector1& other);

    /**
     * Copy one element from a matrix of world vectors.
     */
    wposition1(const wvector& other, size_t row, size_t col);

public:

    //******************************
    // Altitude property

    /**
     * Retrieves the altitude above the mean sea level.
     * Acts as a convenient transformation of the "Rho" property.
     *
     * @return            Altitude in meters.
     */
    inline double altitude() const {
        return rho() - wposition::earth_radius;
    }

    /**
     * Defines the altitude above the mean sea level.
     * Acts as a convenient transformation of the "Rho" property.
     *
     * @param  altitude    Altitude in meters.
     */
    inline void altitude(double altitude) {
        rho(altitude + wposition::earth_radius);
    }

    //******************************
    // Latitude property

    /**
     * Retrieves the latitude component of geodetic earth coordinates.
     * Acts as a convenient transformation of the "Theta" property.
     *
     * @return          Latitude component in degrees.
     */
    inline double latitude() const {
        return to_latitude(theta());
    }

    /**
     * Defines the latitude component of geodetic earth coordinates.
     * Acts as a convenient transformation of the "Theta" property.
     *
     * @param  latitude    Latitude component in degrees.
     */
    inline void latitude(double latitude) {
        theta(to_colatitude(latitude));
    }

    //******************************
    // Longitude property

    /**
     * Retrieves the longitude component of geodetic earth coordinates.
     * Acts as a convenient transformation of the "Phi" property.
     *
     * @return            Longitude component in degrees.
     */
    inline double longitude() const {
        return to_degrees(phi());
    }

    /**
     * Defines the longitude component of geodetic earth coordinates.
     * Acts as a convenient transformation of the "Phi" property.
     *
     * @param  longitude    Longitude component in degrees.
     */
    inline void longitude(double longitude) {
        phi(to_radians(longitude));
    }

    //******************************
    // Utilities

    /**
     * Computes the great circle range and bearing between two
     * latitude/longitude positions along the Earth's surface.
     *
     * @xref E. Williams, "Aviation Formulary V1.43",
     * http://williams.best.vwh.net/avform.htm , July 2010.
     *
     * @param  location     Location of the reference point.
     * @param  bearing      True bearing to the new point (radians, optional).
     * @return              Distance to the new point along the Earth's surface (meters).
     */
    double gc_range( const wposition1& location, double* bearing=nullptr) const ;

};

/// @}
} // end of ocean namespace
} // end of usml namespace
