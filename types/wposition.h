/**
 * @file wposition.h
 * World location in geodetic earth coordinates (latitude, longitude, and altitude).
 */
#pragma once

#include <usml/ublas/ublas.h>
#include <usml/types/wvector.h>

namespace usml {
namespace types {
/// @ingroup wposition
/// @{

/**
 * World location in geodetic earth coordinates (latitude, longitude,
 * and altitude). WGS-84 is used to define a spherical earth system
 * that approximates the earth's radius of curvature across the
 * area of operations.  Higher order geoid schemes (such as EGM-96)
 * are assumed to have minimal impact on the local radius of curvature.
 *
 * The WGS-84 latitude is defined by the angle made between the normal
 * vector on the earth's reference ellipse and the equatorial plane.
 * When traced back to the axis of rotation, this surface normal also
 * passes through the center of curvature.
 *
 * At the equator, the earth's radius of curvature is equal to the WGS-84
 * semi-major axis value of 6378137.0 meters.  As you approach the poles, the
 * radius of curvature gets larger (the earth get flatter) even
 * though the actual radius of the earth gets smaller.  The center of
 * curvature, which is also the center of this model's spherical earth
 * coordinate system, lies on the axis of rotation on the opposite side
 * of the equatorial plane from the area of operations.
 *
 * @xref WGS 84 IMPLEMENTATION MANUAL, Version 2.4, 1998.
 *       See http://www.dqts.net/wgs84.htm for more information.
 */
class USML_DECLSPEC wposition: public wvector
{

public:

    /**
     * Constructs a matrix of wpositions. Initializes each wposition to
     * the surface of the earth at a latitude/longitude of (0,0).
     *
     * @param  rows         Number of rows
     * @param  cols         Number of columns
     * @param  latitude        Initial latitude
     * @param  longitude    Initial longitude
     * @param  altitude        Initial altitude
     */
    wposition(size_t rows = 1, size_t cols = 1, double latitude = 0.0,
            double longitude = 0.0, double altitude = 0.0);

    /**
     * Constructs a new wposition as a copy of an existing wposition.
     * Accepts either an actual wposition, or one of its superclasses.
     *
     * @param  other        wposition to be copied.
     */
    wposition(const wvector & other) :
        wvector(other)
    {
    }

    /**
     * Constructs a mesh of lat/long wpositions. Each row of the mesh
     * corresponds to a single latitude from the input list. Each column
     * of the mesh corresponds to a single longitude from the input list.
     * A common altitude is copied to every point in the mesh.
     *
     * @param  latitude        List of latitudes
     * @param  longitude    List of longitudes
     * @param  altitude        Initial altitude
     */
    template<class E1, class E2> inline wposition(
            const vector_expression<E1>& latitude,
            const vector_expression<E2>& longitude, double altitude = 0.0) :
        wvector(latitude().size(), longitude().size())
    {
        for (int n = 0; n < latitude().size(); ++n) {
            for (int m = 0; m < longitude().size(); ++m) {
                this->latitude(n, m, latitude()(n));
                this->longitude(n, m, longitude()(m));
                this->altitude(n, m, altitude);
            }
        }
    }

    //******************************
    // Earth Radius static property

    /**
     * Radius of curvature, FAI standard for aviation records.
     * @xref Aviation Formulary, V1.46, 2011.
     *       See http://williams.best.vwh.net/avform.htm
     *       for more information.
     */
    static double earth_radius;

    /**
     * Compute the average radius of curvature as a combination of
     * the meridonal radius (rm) and the prime vertical radius (rv).
     * Uses WGS-84 parameters for the semi-major axis and flatting
     * of the Earth.
     * <pre>
     *      a = 6378137.0
     *      f = 1 / 298.257223563
     *      e^2 = f ( 2 - f )
     *      w^2 = 1 - e^2 sin^2(latitude)
     *      rm = a * (1-e^2) / (w*w*w) ;
     *      rv = a / w
     *      R = sqrt( rm rv )
     * </pre>
     * Results are stored in the earth_radius static variable.
     *
     * @param latitude        Latitude component in degrees.
     *
     * @xref J. G. Dworski, J. A. Mercer, "Hamiltonian 3-D Ray Tracing
     * in the Oceanic Waveguide on the Ellipsoidal Earth", ARL-UW TR8929
     * (Dec 1990)
     */
    static void compute_earth_radius(double latitude);

    // ******************************
    // Altitude property (includes both matrix and indexed accessors)

    /**
     * Retrieves the altitude above the mean sea level.
     * Acts as a convenient transformation of the "Rho" property.
     *
     * @return            Altitude in meters returned as a reference to
     *                  a temporary varible.  The calling routine should
     *                  make a copy of this as soon as possible.
     */
    inline matrix<double> altitude() const
    {
        return rho() - earth_radius;
    }

    /**
     * Defines the altitude above the mean sea level.
     * Acts as a convenient transformation of the "Rho" property.
     *
     * @param  altitude    Altitude in meters.
     * @param  no_alias Use uBLAS noalias() assignment speed-up if true.
     */
    template<class E> inline
    void altitude(const matrix_expression<E>& altitude, bool no_alias = true)
    {
        rho(altitude + earth_radius, no_alias);
    }

    /**
     * Retrieves ia single altitude above the mean sea level.
     * Acts as a convenient transformation of the "Rho" property.
     *
     * @param  row        Row index of the element to access.
     * @param  col        Column index of the element to access.
     * @return            Altitude in meters.
     */
    inline double altitude(size_t row, size_t col) const
    {
        return rho(row, col) - earth_radius;
    }

    /**
     * Defines a single altitude above the mean sea level.
     * Acts as a convenient transformation of the "Rho" property.
     *
     * @param  row          Row index of the element to access.
     * @param  col          Column index of the element to access.
     * @param  altitude     Altitude in meters.
     */
    inline void altitude(size_t row, size_t col, double altitude)
    {
        rho(row, col, altitude + earth_radius);
    }

    //******************************
    // Latitude property (includes both matrix and indexed accessors)

    /**
     * Retrieves the latitude component of geodetic earth coordinates.
     * Acts as a convenient transformation of the "Theta" property.
     *
     * @return          Latitude component in degrees returned as a reference
     *                  to a temporary varible.  The calling routine should
     *                  make a copy of this as soon as possible.
     */
    inline matrix<double> latitude() const
    {
        return to_latitude(theta());
    }

    /**
     * Defines the latitude component of geodetic earth coordinates.
     * Acts as a convenient transformation of the "Theta" property.
     *
     * @param  latitude    Latitude component in degrees.
     * @param  no_alias Use uBLAS noalias() assignment speed-up if true.
     */
    template<class E> inline
    void latitude(const matrix_expression<E>& latitude, bool no_alias = true)
    {
        theta(to_colatitude(latitude), no_alias);
    }

    /**
     * Retrieves a single latitude component of geodetic earth coordinates.
     * Acts as a convenient transformation of the "Theta" property.
     *
     * @param  row          Row index of the element to access.
     * @param  col          Column index of the element to access.
     * @return              Latitude component in degrees.
     */
    inline double latitude(size_t row, size_t col) const
    {
        return to_latitude(theta(row, col));
    }

    /**
     * Defines a single latitude component of geodetic earth coordinates.
     * Acts as a convenient transformation of the "Theta" property.
     *
     * @param  row          Row index of the element to access.
     * @param  col          Column index of the element to access.
     * @param  latitude        Latitude component in degrees.
     */
    inline void latitude(size_t row, size_t col, double latitude)
    {
        theta(row, col, to_colatitude(latitude));
    }

    // ******************************
    // Longitude property (includes both matrix and indexed accessors)

    /**
     * Retrieves the longitude component of geodetic earth coordinates.
     * Acts as a convenient transformation of the "theta" property.
     *
     * @return            Longitude component in degrees returned as a reference
     *                  to a temporary varible.  The calling routine should
     *                  make a copy of this as soon as possible.
     */
    inline matrix<double> longitude() const
    {
        return to_degrees(phi());
    }

    /**
     * Defines the longitude component of geodetic earth coordinates.
     * Acts as a convenient transformation of the "theta" property.
     *
     * @param  longitude    Longitude component in degrees.
     * @param  no_alias     Use uBLAS noalias() assignment speed-up if true.
     */
    template<class E> inline
    void longitude(const matrix_expression<E>& longitude, bool no_alias = true)
    {
        phi(to_radians(longitude), no_alias);
    }

    /**
     * Retrieves a single longitude component of geodetic earth coordinates.
     * Acts as a convenient transformation of the "theta" property.
     *
     * @param  row          Row index of the element to access.
     * @param  col          Column index of the element to access.
     * @return              Longitude component in degrees.
     */
    inline double longitude(size_t row, size_t col) const
    {
        return to_degrees(phi(row, col));
    }

    /**
     * Defines a single longitude component of geodetic earth coordinates.
     * Acts as a convenient transformation of the "theta" property.
     *
     * @param  row          Row index of the element to access.
     * @param  col          Column index of the element to access.
     * @param  longitude        Longitude component in degrees.
     */
    inline void longitude(size_t row, size_t col, double longitude)
    {
        phi(row, col, to_radians(longitude));
    }

};

/// @}
} // end of ocean namespace
} // end of usml namespace
