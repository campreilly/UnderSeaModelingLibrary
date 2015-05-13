/**
 * @file wvector.h
 * World vector in spherical earth coordinates.
 */
#pragma once

#include <usml/ublas/ublas.h>

namespace usml {
namespace types {

using namespace usml::ublas;

class wvector1; // forward reference
class wposition1; // forward reference

/// @ingroup wposition
/// @{

/**
 * World vector in spherical earth coordinates.  
 * Each of the three coordinate parameters (rho, theta, phi) is a 
 * 2-D matrix so that the WaveQ3D model can perform ray tracing operations
 * using fast uBLAS operations across the entire wavefront.
 *
 * Two different coordinate systems use this class as their underlying type:
 *
 *     - The spherical earth coordinate system measures absolute values of 
 *       rho, theta, phi relative to a fixed geocentric reference system.
 *
 *     - The ray direction coordinate system measures values relative 
 *       the local basis vectors for rho, theta, phi.  These basis vectors
 *       change based on the current location of the ray.
 *
 * @xref Wikipedia Article on "Geodetic System"
 *       http://en.wikipedia.org/wiki/Geodetic_system
 *
 * @xref Portland State Aerospace Society, "Local Tangent Plane",
 *       Version 2.01 (2007.9.15), http://www.psas.pdx.edu.
 */
class USML_DECLSPEC wvector
{

public:

    //*********************************
    // initialization

    /**
     * Constructs a new coordinate from matrix dimensions.
     *
     * @param  rows    Number of rows
     * @param  cols    Number of columns
     */
    wvector(size_t rows = 1, size_t cols = 1);

    //*********************************
    // Rho property (includes both matrix and indexed accessors)

private:

    /** Storage for the radial component of the coordinate system. */
    matrix<double> _rho;

public:

    /**
     * Retrieves the radial component of the spherical earth coordinate system.
     * Measured as the distance from the center of curvature for the area of
     * operations.  Because the shape of the earth is elliptical, this is not
     * the same as the distance from the center of the earth.
     *
     * @return            Radial coordinate in meters.
     */
    inline const matrix<double>& rho() const
    {
        return _rho;
    }

    /**
     * Defines the radial component of the spherical earth coordinate.
     *
     * @param  r        Radial coordinate in meters.
     * @param  no_alias Use uBLAS noalias() assignment speed-up if true.
     */
    template<class E> inline
    void rho(const matrix_expression<E>& r, bool no_alias = true)
    {
        if (no_alias) {
            noalias(_rho) = r;
        } else {
            _rho = r;
        }
    }

    /**
     * Indexed version for single radial component retrieval.
     *
     * @param  row      Row index of the element to access.
     * @param  col      Column index of the element to access.
     * @return          Radial coordinate in meters.
     */
    inline double rho(size_t row, size_t col) const
    {
        return _rho(row, col);
    }

    /**
     * Indexed version for single radial component definition.
     *
     * @param  row      Row index of the element to access.
     * @param  col      Column index of the element to access.
     * @param  r        Radial coordinate in meters.
     *
     */
    inline void rho(size_t row, size_t col, double r)
    {
        _rho(row, col) = r;
    }

    //*********************************
    // Theta property (includes both matrix and indexed accessors)

private:

    /** Storage for the colatitude component of the coordinate system. */
    matrix<double> _theta;

public:

    /**
     * Retrieves the colatitude component of the spherical earth 
     * coordinate system.  Measured as the angle down from the north pole.
     * This property is the complement of latitude, which is measured 
     * up from the equator.
     *
     * @return            Colatitude coordinate in radians.
     */
    inline const matrix<double>& theta() const
    {
        return _theta;
    }

    /**
     * Defines the colatitude component of the spherical earth coordinate.
     *
     * @param  t        Colatitude coordinate in radians.
     * @param  no_alias Use uBLAS noalias() assignment speed-up if true.
     */
    template<class E> inline
    void theta(const matrix_expression<E>& t, bool no_alias = true)
    {
        if (no_alias) {
            noalias(_theta) = t;
        } else {
            _theta = t;
        }
    }

    /**
     * Indexed version for single colatitude component retrieval.
     *
     * @param  row      Row index of the element to access.
     * @param  col      Column index of the element to access.
     * @return            Colatitude coordinate in radians.
     */
    inline double theta(size_t row, size_t col) const
    {
        return _theta(row, col);
    }

    /**
     * Indexed version for single colatitude component definition.
     * Caches the sine of the colatitude as a side effect.
     *
     * @param  row      Row index of the element to access.
     * @param  col      Column index of the element to access.
     * @param  t        Colatitude coordinate in radians.
     *
     */
    inline void theta(size_t row, size_t col, double t)
    {
        _theta(row, col) = t;
    }

    //*********************************
    // Phi property (includes both matrix and indexed accessors)

private:

    /** Storage for the longitude component of the coordinate system. */
    matrix<double> _phi;

public:

    /**
     * Retrieves the longitude component of the spherical earth 
     * coordinate system.  Measured as the angle east from the 
     * prime meridian.
     *
     * @return          Longitude coordinate in radians.
     */
    inline const matrix<double>& phi() const
    {
        return _phi;
    }

    /**
     * Defines the longitude component of the spherical earth coordinate.
     *
     * @param  p        Longitude coordinate in radians.
     * @param  no_alias Use uBLAS noalias() assignment speed-up if true.
     */
    template<class E> inline
    void phi(const matrix_expression<E>& p, bool no_alias = true)
    {
        if (no_alias) {
            noalias(_phi) = p;
        } else {
            _phi = p;
        }
    }

    /**
     * Indexed version for single longitude component retrieval.
     *
     * @param  row      Row index of the element to access.
     * @param  col      Column index of the element to access.
     * @return          Longitude coordinate in radians.
     */
    inline double phi(size_t row, size_t col) const
    {
        return _phi(row, col);
    }

    /**
     * Indexed version for single longitude component definition.
     *
     * @param  row      Row index of the element to access.
     * @param  col      Column index of the element to access.
     * @param  p        Longitude coordinate in radians.
     *
     */
    inline void phi(size_t row, size_t col, double p)
    {
        _phi(row, col) = p;
    }

    //*********************************
    // utilities

    /**
     * Number of rows in each coordinate.
     */
    inline size_t size1() const
    {
        return _rho.size1();
    }

    /**
     * Number of columns in each coordinate.
     */
    inline size_t size2() const
    {
        return _rho.size2();
    }

    /**
     * Reset all data elements back to zero.
     */
    inline void clear()
    {
        _rho.clear();
        _theta.clear();
        _phi.clear();
    }

    /**
     * Compute the dot product between this vector and some other 
     * spherical earth vector.  The transformation from cartesian
     * to spherical earth coordinate shows that:
     * <pre>
     *      dot product 
     *          = x1*x2 + y1*y2 + z1*z2
     *          = r1*r2 * ( cos(t1)cos(t2) + sin(t1)sin(t2)cos(p1-p2) )
     * </pre>
     * This implementation uses the haversine formula to avoid
     * errors in the dot product when distances are small.
     * <pre>
     *      dot product = r1*r2
     *          * { 1-2*{ sin^2[(t1-t2)/2] + sin(t1)sin(t2)sin^2[(p1-p2)/2] } }
     * </pre>
     * @param  other    Second vector in the dot product.
     * @param  result   Dot product between these two vectors.
     *
     * @xref Weisstein, Eric W. "Spherical Trigonometry."
     * From MathWorld--A Wolfram Web Resource.
     * http://mathworld.wolfram.com/SphericalTrigonometry.html.
     * @xref E. Williams, "Aviation Formulary V1.46",
     * http://williams.best.vwh.net/avform.htm
     */
    void dot(const wvector1& other, matrix<double>& result) const;

    /**
     * Dot product divided by the magnitude of the two vectors.
     * Used to compute the angle between vectors.
     * Uses the haversine formula to avoid errors for short distances.
     * <pre>
     *      cos(angle) = dot(v1,v2) / ( |v1| |v2| )
     *                 = ( cos(t1)cos(t2) + sin(t1)sin(t2)cos(p1-p2) )
     *                 = { 1-2*( sin^2[(t1-t2)/2] + sin(t1)sin(t2)sin^2[(p1-p2)/2] ) }
     * </pre>
     * @param  other    Second vector in the dot product.
     * @param  result   Normalized dot product between these two vectors.
     */
    void dotnorm(const wvector1& other, matrix<double>& result) const;

    /**
     * Compute the straight line distance between this point and
     * some other point in space.  The distance is related
     * to the dot product by the equation:
     * <pre>
     *      distance = sqrt( r1*r1 + r2*r2 - 2 dot(r1,r2) )
     * </pre>
     * This as the general form of Pythagoras' theorem. 
     *
     * @param  origin   Point from which distance is measured.
     * @param  result   Distance from this point to the origin point.
     */
    void distance(const wvector1& origin, matrix<double>& result) const;

    /**
     * Compute the square of the straight line distance between this point and
     * some other point in space.  The square of the distance is related
     * to the dot product by the equation:
     * <pre>
     *      distance^2 = r1*r1 + r2*r2 - 2 dot(r1,r2)
     * </pre>
     * This as the general form of Pythagoras' theorem. 
     *
     * @param  origin   Point from which distance is measured.
     * @param  result   Distance from this point to the origin point.
     */
    void distance2(const wvector1& origin, matrix<double>& result) const;

};

/// @}
} // end of ocean namespace
} // end of usml namespace
