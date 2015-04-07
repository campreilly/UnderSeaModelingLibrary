/**
 * @file orientation.cc
 */
#include <usml/sensors/orientation.h>
#include <boost/numeric/ublas/io.hpp>

using namespace usml::sensors ;

/**
 * Default constructor
 */
orientation::orientation()
    : _heading(0.0), _pitch(0.0), _roll(0.0),
      _axis(3,0)
{

}

/**
 * Pitch, heading, roll constructor
 */
orientation::orientation(
    double heading, double pitch, double roll,
    vector<double> ref_axis )
    : _heading(-pitch*M_PI/180.0),
      _pitch(-heading*M_PI/180.0),
      _roll(roll*M_PI/180.0),
      _axis(ref_axis)
{
    apply_rotation() ;
}

/**
 * Tilt angle/direction constructor
 * NOTE: This is a dummy constructor at this time
 */
orientation::orientation(
    double angle, double direction, vector<double> ref_axis )
    : _heading(0.0), _pitch(0.0), _roll(0.0), _axis(ref_axis)
{

}

/**
 * Applies a rotation from one coordinate system to the
 * current rotated coordinates for asymmetric systems.
 */
void orientation::apply_rotation(
    double& de, double& az,
    double* de_prime, double* az_prime )
{
    _theta = M_PI_2 - de ;
    _phi = az ;
    _axis(0) = sin(_theta) * cos(_phi) ;
    _axis(1) = sin(_theta) * sin(_phi) ;
    _axis(2) = cos(_theta) ;
    apply_rotation() ;
    *de_prime = M_PI_2 - _theta ;
    *az_prime = _phi ;
}

/**
 * Applies a rotation from one coordinate system to the
 * current rotated coordinates for symmetric systems.
 */
void orientation::apply_rotation()
{
    _x = _axis(0)*cos(_heading)*cos(_roll) +
         _axis(2)*( sin(_heading)*sin(_pitch) + cos(_heading)*cos(_pitch)*sin(_roll) ) +
         _axis(1)*( -cos(_pitch)*sin(_heading) + cos(_heading)*sin(_pitch)*sin(_roll) ) ;
    _y = _axis(0)*cos(_roll)*sin(_heading) +
         _axis(2)*( -cos(_heading)*sin(_pitch) + cos(_pitch)*sin(_heading)*sin(_roll) ) +
         _axis(1)*( cos(_heading)*cos(_pitch) + sin(_heading)*sin(_pitch)*sin(_roll) ) ;
    _z = _axis(2)*cos(_pitch)*cos(_roll) +
         _axis(1)*cos(_roll)*sin(_pitch) -
         _axis(0)*sin(_roll) ;
    convert_to_spherical() ;
}

/**
 * Converts the store vector in spherical coordinates to
 * cartesian coordinates.
 */
void orientation::convert_to_cartesian()
{
    _x = sin(_theta) * cos(_phi) ;
    _y = sin(_theta) * sin(_phi) ;
    _z = cos(_theta) ;
}

/**
 * Converts the stored vector in cartesian coordinates to
 * spherical coordinates.
 */
void orientation::convert_to_spherical()
{
//    double rho = std::sqrt( _x*_x + _y*_y + _z*_z ) ;
    _theta = std::acos( _z - 1e-10 ) ;
    _theta = std::fmod( _theta, M_PI ) ;
    _phi = std::atan2( _y, _x ) ;
    _phi = std::fmod( _phi, 2.0*M_PI ) ;
}
