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
    : _heading(0.0), _pitch(0.0), _roll(0.0)
{

}

/**
 * Pitch, heading, roll constructor
 */
orientation::orientation(
    double heading, double pitch, double roll )
    : _heading(-pitch*M_PI/180.0),
      _pitch(-heading*M_PI/180.0),
      _roll(roll*M_PI/180.0)
{

}

/**
 * Tilt angle/direction constructor
 * NOTE: This is a dummy constructor at this time
 */
orientation::orientation(
    double angle, double direction )
    : _heading(0.0), _pitch(0.0), _roll(0.0)
{

}

/**
 * Applies a rotation from one coordinate system to the
 * current rotated coordinates.
 */
void orientation::apply_rotation(
    const vector<double> ref_axis,
    double& theta, double& phi  )
{
    _v_cart(0) = ref_axis(0)*cos(_heading)*cos(_roll) +
                 ref_axis(2)*( sin(_heading)*sin(_pitch) + cos(_heading)*cos(_pitch)*sin(_roll) ) +
                 ref_axis(1)*( -cos(_pitch)*sin(_heading) + cos(_heading)*sin(_pitch)*sin(_roll) ) ;
    _v_cart(1) = ref_axis(0)*cos(_roll)*sin(_heading) +
                 ref_axis(2)*( -cos(_heading)*sin(_pitch) + cos(_pitch)*sin(_heading)*sin(_roll) ) +
                 ref_axis(1)*( cos(_heading)*cos(_pitch) + sin(_heading)*sin(_pitch)*sin(_roll) ) ;
    _v_cart(2) = ref_axis(2)*cos(_pitch)*cos(_roll) +
                 ref_axis(1)*cos(_roll)*sin(_pitch) -
                 ref_axis(0)*sin(_roll) ;
    convert_to_spherical() ;
    theta = std::fmod( _v(1), M_PI ) ;
    phi = std::fmod( _v(2), 2.0*M_PI ) ;
}

/**
 * Converts the store vector in spherical coordinates to
 * cartesian coordinates.
 */
void orientation::convert_to_cartesian()
{
    _v_cart(0) = _v(0) * sin(_v(1)) * cos(_v(2)) ;
    _v_cart(1) = _v(0) * sin(_v(1)) * sin(_v(2)) ;
    _v_cart(2) = _v(0) * cos(_v(1)) ;
}

/**
 * Converts the stored vector in cartesian coordinates to
 * spherical coordinates.
 */
void orientation::convert_to_spherical()
{
    _v(0) = std::sqrt( _v_cart(0)*_v_cart(0)
                     + _v_cart(1)*_v_cart(1)
                     + _v_cart(2)*_v_cart(2) ) ;
    _v(1) = std::acos( _v_cart(2) / _v(0) ) ;
    _v(2) = std::atan2( _v_cart(1), _v_cart(0) ) ;
}
