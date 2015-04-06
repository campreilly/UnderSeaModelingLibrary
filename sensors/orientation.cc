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
    : _pitch(0.0), _heading(0.0), _roll(0.0),
      _rx(3,3), _ry(3,3), _rz(3,3), _rotation(3,3)
{
    initialize_matrices() ;
}

/**
 * Pitch, heading, roll constructor
 */
orientation::orientation(
    double pitch, double heading, double roll )
    : _pitch(pitch), _heading(heading), _roll(roll),
      _rx(3,3), _ry(3,3), _rz(3,3), _rotation(3,3)
{
    initialize_matrices() ;
}

/**
 * Tilt angle/direction constructor
 * NOTE: This is a dummy constructor at this time
 */
orientation::orientation(
    double angle, double direction )
    : _pitch(0.0), _heading(0.0), _roll(0.0),
      _rx(3,3), _ry(3,3), _rz(3,3), _rotation(3,3)
{
    initialize_matrices() ;
}

/**
 * Applies a rotation from one coordinate system to the
 * current rotated coordinates.
 */
void orientation::apply_rotation(
    const vector<double> ref_axis,
    double& theta, double& phi  )
{
    _v_cart = prod( _rotation, ref_axis ) ;
    convert_to_spherical() ;
    theta = std::fmod( _v(1), M_PI ) ;
    phi = std::fmod( _v(2), 2.0*M_PI ) ;
}

/**
 * Sets the rotational invariant components of the rotation
 * matrices and then constructs the full rotation matrix.
 */
void orientation::initialize_matrices()
{
    _v.clear() ;
    _v_cart.clear() ;
    _v(0) = 1.0 ;
    // set the rotation invariant components of the matrices
    _rx.clear() ;
    _rx(0,0) = 1.0 ;
    _ry.clear() ;
    _ry(1,1) = 1.0 ;
    _rz.clear() ;
    _rz(2,2) = 1.0 ;
    _rotation.clear() ;
    compute_rotation_matrix() ;
}

/**
 * Computes the inverse rotation matrix needed to transform
 * incoming angles to the newly rotated coordinate system.
 */
void orientation::compute_rotation_matrix()
{
    // x-axis rotation
    _rx(1,1) = _rx(2,2) = cos(_roll) ;
    _rx(1,2) = -sin(_roll) ;
    _rx(2,1) = -_rx(1,2) ;
    // y-axis rotation
    _ry(0,0) = _ry(2,2) = cos(_pitch) ;
    _ry(0,2) = sin(_pitch) ;
    _ry(2,0) = -_ry(0,2) ;
    // z-axis rotation
    _rz(0,0) = _rz(1,1) = cos(_heading) ;
    _rz(0,1) = -sin(_heading) ;
    _rz(1,0) = -_rz(0,1) ;
    // Full inverse rotation matrix
    matrix<double> tmp = prod( _ry, _rx ) ;
    noalias(_rotation) = prod( _rz, tmp ) ;
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
