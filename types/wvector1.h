/**
 * @file wvector1.h
 * Individual world vector in spherical earth coordinates.
 */
#pragma once

#include <usml/types/wvector.h>
#include <usml/ublas/ublas.h>

namespace usml {
namespace types {

using namespace usml::ublas;

class wvector1;  // forward reference

/// @ingroup wposition
/// @{

/**
 * Individual world vector in spherical earth coordinates.
 * Each of the three coordinate parameters (rho,theta,phi) is a
 * scalar.  This class is a convenience to save the developer
 * from doing a lot of operations on 1x1 matrices.
 */
class USML_DECLSPEC wvector1 {
   public:
    //*********************************
    // initialization

    /**
     * Constructs an individual vector.
     */
    wvector1(double r = 0.0, double t = 0.0, double p = 0.0);

    /**
     * Copy one element from a matrix of world vectors.
     */
    wvector1(const wvector& other, size_t row, size_t col);

    //*********************************
    // Rho property

   private:
    /** Storage for the radial component of the coordinate system. */
    double _rho;

   public:
    /**
     * Retrieves the radial component of the spherical earth coordinate system.
     *
     * @return            Radial coordinate in meters.
     */
    inline double rho() const { return _rho; }

    /**
     * Defines the radial component of the spherical earth coordinate.
     *
     * @param  r        Radial coordinate in meters.
     */
    inline void rho(double r) { _rho = r; }

    //*********************************
    // Theta property

   private:
    /** Storage for the colatitude component of the coordinate system. */
    double _theta;

   public:
    /**
     * Retrieves the colatitude component of the spherical earth
     * coordinate system.
     *
     * @return            Colatitude coordinate in radians.
     */
    inline double theta() const { return _theta; }

    /**
     * Defines the colatitude component of the spherical earth coordinate.
     *
     * @param  t        Colatitude coordinate in radians.
     */
    inline void theta(double t) { _theta = t; }

    //*********************************
    // Phi property

   private:
    /** Storage for the longitude component of the coordinate system. */
    double _phi;

   public:
    /**
     * Retrieves the longitude component of the spherical earth
     * coordinate system.
     *
     * @return          Longitude coordinate in radians.
     */
    inline double phi() const { return _phi; }

    /**
     * Defines the longitude component of the spherical earth coordinate.
     *
     * @param  p        Longitude coordinate in radians.
     */
    inline void phi(double p) { _phi = p; }

    //*********************************
    // utilities

    /**
     * Reset all data elements back to zero.
     */
    void clear();

    /**
     * Compute the dot product between this vector and some other
     * spherical earth vector.
     *
     * @param  other    Second vector in the dot product.
     * @return          Dot product between these two vectors.
     */
    double dot(const wvector1& other) const;

    /**
     * Dot product divided by the magnitude of the two vectors.
     *
     * @param  other    Second vector in the dot product.
     * @return          Normalized dot product between these two vectors.
     */
    double dotnorm(const wvector1& other) const;

    /**
     * Compute the straight line distance between this point and
     * some other point in space.
     *
     * @param  origin   Point from which distance is measured.
     * @return          Distance from this point to the origin point.
     */
    double distance(const wvector1& origin) const;

    /**
     * Compute the square of the straight line distance between this point and
     * some other point in space.
     *
     * @param  origin   Point from which distance is measured.
     * @return          Distance from this point to the origin point.
     */
    double distance2(const wvector1& origin) const;

    /**
     * Extract depression/elevation and azimuthal angles from
     * acoustic ray direction in the local tangent plane.
     * Used to extract ray angle information from eigenray
     * collisions with targets.
     *
     * @param  de           Initial depression/elevation angle at the
     *                      source location (degrees, positive is up).
     * @param  az           Initial azimuthal angle at the source location
     *                      (degrees, clockwise from true north).
     */
    void direction(double* de, double* az) const;

    /**
     * Compute acoustic ray direction in the local tangent plane using
     * the spherical earth geocentric direction vector.  Used to
     * re-initialize the direction of propagation after reflection.
     *
     * @param  pos          Position from which direction is measured.
     * @param  dir          Direction in terms of a spherical earth vector
     *                      (input).
     * @return              Length of dir vector in local tangent plane.
     */
    double direction(const wvector1& pos, const wvector1& dir);

    /**
     * Extract spherical earth geocentric direction from acoustic
     * ray direction in the local tangent plane. Used to extract
     * grazing angle information at the beginning of the reflection
     * process.
     *
     * @param  pos          Position from which direction is measured.
     * @param  dir          Direction in terms of a spherical earth vector
     *                      (output).
     */
    void direction(const wvector1& pos, wvector1* dir) const;

    /**
     * Compute the surface area between three points in space.  The distances
     * between the vertices in the triangle (this,p2,p3) can be used to
     * compute the surface area using the formula:
     * <pre>
     *         a = length(this,p2)
     *         b = length(this,p3)
     *         c = length(p2,p3)
     *      area = 1/4 sqrt[ (a+b+c)(b+c-a)(c+a-b)(a+b-c) ]
     * </pre>
     *
     * @xref Weisstein, Eric W. "Triangle Area." From MathWorld - A Wolfram
     *       Web Resource. http://mathworld.wolfram.com/TriangleArea.html
     *
     * @param   p2   Second point in space
     * @param   p3   Third point in space
     */
    double area(const wvector1& p2, const wvector1& p3) const;

    /**
     * Compute the surface area between four points in space. Similar
     * to the three point version, but this one combines two triangle
     * (this,p2,p3) and (this,p3,p4) to form the complete solution.
     * This allows the distance from this to p3 to be reused.
     *
     * @param   p2   Second point in space
     * @param   p3   Third point in space
     * @param   p4   Forth point in space
     */
    double area(const wvector1& p2, const wvector1& p3,
                const wvector1& p4) const;
};

/// @}
}  // namespace types
}  // namespace usml
