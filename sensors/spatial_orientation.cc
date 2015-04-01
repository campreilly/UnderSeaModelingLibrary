/**
 * @file spatial_orientation.cc
 */
#include <usml/sensors/spatial_orientation.h>
#include <boost/numeric/ublas/io.hpp>

using namespace usml::sensors ;

/**
 * Default constructor
 */
spatial_orientation::spatial_orientation()
    : _pitch(0.0), _heading(0.0), _roll(0.0),
      _axis(spatial_orientation::VERTICAL),
      _rx(3,3), _ry(3,3), _rz(3,3), _rotation(3,3)
{
    initialize_matrices() ;
}

/**
 * Pitch, heading, roll constructor
 */
spatial_orientation::spatial_orientation(
    double pitch, double heading, double roll, reference_axis axis )
    : _pitch(pitch), _heading(heading), _roll(roll), _axis(axis),
      _rx(3,3), _ry(3,3), _rz(3,3), _rotation(3,3)
{
//    if( _axis == HORIZONTAL ) {
//        _heading = heading - M_PI_2 ;
//    }
    initialize_matrices() ;
}

/**
 * Tilt angle/direction constructor
 * NOTE: This is a dummy constructor at this time
 */
spatial_orientation::spatial_orientation(
    double angle, double direction, reference_axis axis )
    : _pitch(0.0), _heading(0.0), _roll(0.0), _axis(axis),
      _rx(3,3), _ry(3,3), _rz(3,3), _rotation(3,3)
{
    initialize_matrices() ;
}

/**
 * Applies a rotation from one coordinate system to the
 * current rotated coordinates.
 */
void spatial_orientation::apply_rotation(
    const vector<double> ref_axis,
    double& theta, double& phi  )
{
//    std::cout << "_rotation: " << _rotation << std::endl ;
//    _v = ref_axis ;
//    std::cout << "_v: " << _v << std::endl ;
//    convert_to_cartesian() ;
//    std::cout << "_v_cart: " << _v_cart << std::endl ;
//    _v_cart = prod( _rotation, _v_cart ) ;
    _v_cart = prod( _rotation, ref_axis ) ;
//    std::cout << "_v_cart: " << _v_cart << std::endl ;
    convert_to_spherical() ;
//    std::cout << "_v: " << _v << std::endl ;
    theta = _v(1) ;
//    theta_prime = std::fmod(_v(1) + M_PI_2, M_PI) ;
    phi = _v(2) ;
}

/**
 * Sets the rotational invariant components of the rotation
 * matrices and then constructs the full rotation matrix.
 */
void spatial_orientation::initialize_matrices()
{
    _v(0) = 1.0 ;
    // set the rotation invariant components of the matrices
    _rx.clear() ;
    _rx(0,0) = 1.0 ;
    _ry.clear() ;
    _ry(1,1) = 1.0 ;
    _rz.clear() ;
    _rz(2,2) = 1.0 ;
    _rotation.clear() ;
    compute_inverse_matrix() ;
}

/**
 * Computes the inverse rotation matrix needed to transform
 * incoming angles to the newly rotated coordinate system.
 */
void spatial_orientation::compute_inverse_matrix()
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
void spatial_orientation::convert_to_cartesian()
{
    _v_cart(0) = _v(0) * sin(_v(1)) * cos(_v(2)) ;
    _v_cart(1) = _v(0) * sin(_v(1)) * sin(_v(2)) ;
    _v_cart(2) = _v(0) * cos(_v(1)) ;
}

/**
 * Converts the stored vector in cartesian coordinates to
 * spherical coordinates.
 */
void spatial_orientation::convert_to_spherical()
{
    _v(0) = std::sqrt( _v_cart(0)*_v_cart(0)
                     + _v_cart(1)*_v_cart(1)
                     + _v_cart(2)*_v_cart(2) ) ;
    _v(1) = std::acos( _v_cart(2) / _v(0) ) ;
    _v(2) = std::atan2( _v_cart(1), _v_cart(0) ) ;
}
