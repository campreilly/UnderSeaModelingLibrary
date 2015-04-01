/**
 * @file spatial_orientation.h
 */
#pragma once

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <usml/usml_config.h>

using namespace boost::numeric::ublas ;

namespace usml {
namespace sensors {

/**
 * This class holds the physical orientation of the
 * sensor and provides a conversion from incident ray
 * coordinates to the spherical array coordinates.
 *
 * Pitch - the amount of rotation about the local z-axis
 * Heading - the amount of rotation about the local y-axis
 * Roll - the amount of rotation about the local x-axis
 */
class USML_DECLSPEC spatial_orientation {

public:

    typedef enum { VERTICAL, HORIZONTAL } reference_axis ;

    /**
     * Default constructor
     */
    spatial_orientation() ;

    /**
     * Constructor using a given pitch, heading, and roll
     *
     * @param pitch     rotation about the z-axis (radians)
     * @param heading   rotation about the y-axis (radians)
     * @param roll      rotation about the x-axis (radians)
     */
    spatial_orientation( double pitch, double heading, double roll, reference_axis axis=VERTICAL ) ;

    /**
     * Constructor using a tile angle/direction????
     */
    spatial_orientation( double angle, double direction, reference_axis axis ) ;

    /**
     * Rotates the incoming coordinates into the current rotated
     * coordinate system.
     *
     * @param theta         rotated theta coordinate (radians)
     * @param phi           rotated phi coordinate (radians)
     */
    void apply_rotation( const vector<double> ref_axis,
                         double& theta, double& phi ) ;

    /**
     * Current pitch of the rotated system.
     * @return  current value stored in _pitch
     */
    double pitch() const {
        return _pitch ;
    }

    /**
     * Updates the pitch for the rotated system.
     * @param p     new pitch of the rotated system
     */
    void pitch( double p ) {
        _pitch = p ;
        compute_inverse_matrix() ;
    }

    /**
     * Current heading of the rotated system.
     * @return  current value stored in _heading
     */
    double heading() {
        return _heading ;
    }

    /**
     * Updates the heading for the rotated system.
     * @param h     new heading of the rotated system
     */
    void heading( double h ) {
        _heading = h ;
        compute_inverse_matrix() ;
    }

    /**
     * Current roll of the rotated system.
     * @return  current value stored in _roll
     */
    double roll() const {
        return _roll ;
    }

    /**
     * Updates the roll for the rotated system.
     * @param r     new roll of the rotated system
     */
    void roll( double r ) {
        _roll = r ;
        compute_inverse_matrix() ;
    }

    /**
     * Update all three rotation angles
     *
     * @param p     new pitch of the rotated system
     * @param h     new heading of the rotated system
     * @param r     new roll of the rotated system
     */
    void update_orientation( double p, double h, double r ) {
        _pitch = p ;
        _heading = h ;
        _roll = r ;
        compute_inverse_matrix() ;
    }

private:

    /**
     * Member variables
     */
    double _pitch ;
    double _heading ;
    double _roll ;
    reference_axis _axis ;

    /**
     * Inverse Rotation Matrices
     */
    matrix<double> _rx ;
    matrix<double> _ry ;
    matrix<double> _rz ;
    matrix<double> _rotation ;

    /**
     * Rotation vector
     */
    c_vector<double,3> _v ;
    c_vector<double,3> _v_cart ;

    /**
     * Computes the inverse rotation matrices
     */
    void compute_inverse_matrix() ;

    /**
     * Initializes the matrices rotation invariant components
     * and then calls the compute_inverse_matrix component to complete
     * the full computation of the inverse rotation matrix
     */
    void initialize_matrices() ;

    /**
     * Convert from Spherical coordinates to Cartesian coordinates
     * This is done prior to applying the inverse rotation matrix
     */
    void convert_to_cartesian() ;

    /**
     * Convert from Cartesian coordinates to Spherical coordinates
     * This is done after applying the inverse rotation matrix and
     * returning the newly rotated coordinates.
     */
    void convert_to_spherical() ;

};

}   // end of namespace sensors
}   // end of namespace usml
