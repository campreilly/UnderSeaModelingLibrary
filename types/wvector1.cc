/*
 * @file wvector1.cc
 * Individual world vector in spherical earth coordinates.
 */
#include <usml/types/wvector1.h>
#include <usml/types/wposition1.h>
#include <usml/types/wposition.h>

using namespace usml::types;

//*********************************
// initialization

/**
 * Constructs an individual vector.
 */
wvector1::wvector1(double r, double t, double p) :
    _rho(r), _theta(t), _phi(p)
{
}

/**
 * Copy one element from a matrix of world vectors.
 */
wvector1::wvector1(const wvector& other, unsigned row, unsigned col) :
    _rho(other.rho(row, col)), _theta(other.theta(row, col)), _phi(other.phi(
            row, col))
{
}

//*********************************
// utilities

/**
 * Reset all data elements back to zero.
 */
void wvector1::clear()
{
    _rho = 0.0;
    _theta = 0.0;
    _phi = 0.0;
}

/**
 * Compute the dot product between this vector and some other
 * spherical earth vector.
 */
double wvector1::dot(const wvector1& other) const
{
    return _rho * other._rho * dotnorm(other) ;
}

/**
 * Dot product divided by the magnitude of the two vectors.
 */
double wvector1::dotnorm(const wvector1& other) const
{
    // uses the same equation as dot() without the radial terms
    const double sin_theta = sin( 0.5*(_theta-other._theta) ) ;
    const double sin_phi = sin( 0.5*(_phi-other._phi) ) ;
    return 1.0 - 2.0 * ( sin_theta*sin_theta + sin(_theta) * sin(other._theta)
            * sin_phi*sin_phi ) ;
}

/**
 * Compute the straight line distance between this point and
 * some other point in space.
 */
double wvector1::distance(const wvector1& origin) const
{
    return sqrt(distance2(origin));
}

/**
 * Compute the square of the straight line distance between this point and
 * some other point in space.
 */
double wvector1::distance2(const wvector1& origin) const {
    return abs( _rho * _rho + origin._rho * origin._rho - 2.0 * dot(origin) );
}

/**
 * Extract depression/elevation and azimuthal angles from
 * acoustic ray direction in the local tangent plane.
 */
void wvector1::direction(double* de, double* az) const
{
    *de = to_degrees(atan2(_rho, sqrt(_theta*_theta + _phi*_phi)));
    *az = to_degrees(atan2(_phi, -_theta));
}

/**
 * Compute acoustic ray direction in the local tangent plane using
 * the spherical earth geocentric direction vector.
 */
void wvector1::direction(const wvector1& pos, const wvector1& dir)
{

    // Wikipedia Article on "Geodetic System" conversion from
    // Earth Centred Earth Fixed (ECEF) coordinates to
    // Local east, north, up (ENU) coordinates

    const double dir_st = sin(dir.theta());
    const double dir_ct = cos(dir.theta());
    const double dir_sp = sin(dir.phi());
    const double dir_cp = cos(dir.phi());

    const double pos_st = sin(dir.theta());
    const double pos_ct = cos(dir.theta());
    const double pos_sp = sin(dir.phi());
    const double pos_cp = cos(dir.phi());

    const double x = dir.rho() * dir_cp * dir_st - pos.rho() * pos_cp * pos_st;
    const double y = dir.rho() * dir_sp * dir_st - pos.rho() * pos_sp * pos_st;
    const double z = dir.rho() * dir_ct - pos.rho() * pos_ct;

    const double e = -pos_sp * x + pos_cp;
    const double n = -pos_ct * pos_cp * x - pos_ct * pos_sp * y + pos_st * z;
    const double u = pos_st * pos_cp * x - pos_st * pos_sp * y + pos_ct * z;

    const double length = sqrt(e * e + n * n + u * u);

    rho(u / length);
    theta(-n / length);
    phi(e / length);
}

/**
 * Extract spherical earth geocentric direction from acoustic
 * ray direction in the local tangent plane.
 */
void wvector1::direction(const wvector1& pos, wvector1* dir)
{
    const double st = sin(pos.theta());
    const double ct = cos(pos.theta());
    const double sp = sin(pos.phi());
    const double cp = cos(pos.phi());

    const double x = _rho * ct * sp - _theta * st + _phi * ct * cp;
    const double y = _rho * st * sp + _theta * ct + _phi * st * cp;
    const double z = _rho * cp - _phi * sp;

    dir->rho(1.0);
    dir->theta(atan2(sqrt(x * x + y * y), z));
    dir->phi(atan2(y, x));
}

/**
 * Compute the surface area between three points in space.
 */
double wvector1::area(const wvector1& p2, const wvector1& p3) const
{
    const double a = distance(p2);
    const double b = distance(p3);
    const double c = p2.distance(p3);
    return 0.25 * sqrt((a + b + c) * (b + c - a) * (c + a - b) * (a + b - c));
}

/**
 * Compute the surface area between four points in space.
 */
double wvector1::area(const wvector1& p2, const wvector1& p3,
        const wvector1& p4) const
{

    // compute area of triangle (this,p2,p3)

    double a = distance(p2);
    const double b = distance(p3);
    double c = p2.distance(p3);
    const double area1 = 0.25 * sqrt((a + b + c) * (b + c - a) * (c + a - b)
            * (a + b - c));
    /*
     cout << "p2-p1=(" << (p2.rho()-rho()) << "," << (p2.theta()-theta())  << "," << (p2.phi()-phi()) << ")" << endl
     << "p3-p1=(" << (p3.rho()-rho()) << "," << (p3.theta()-theta())  << "," << (p3.phi()-phi()) << ")" << endl
     << "p3-p2=(" << (p3.rho()-p2.rho()) << "," << (p3.theta()-p2.theta())  << "," << (p3.phi()-p2.phi()) << ")" << endl
     << "p4-p1=(" << (p4.rho()-rho()) << "," << (p4.theta()-theta())  << "," << (p4.phi()-phi()) << ")" << endl
     << "p3-p4=(" << (p3.rho()-p4.rho()) << "," << (p3.theta()-p4.theta())  << "," << (p3.phi()-p4.phi()) << ")" << endl
     << " d12=" << a
     << " d13=" << b
     << " d23=" << c ;
     */
    // compute area of triangle (this,p3,p4)

    a = distance(p4);
    c = p4.distance(p3);
    const double area2 = 0.25 * sqrt((a + b + c) * (b + c - a) * (c + a - b)
            * (a + b - c));
    /*
     cout << " d14=" << a
     << " d43=" << c
     << " area1=" << area1
     << " area2=" << area2
     << endl ;
     */
    return area1 + area2;
}
