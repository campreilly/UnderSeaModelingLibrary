/**
 * @file orientation.h
 */
#pragma once

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
 * Heading - the amount of rotation about the z-axis
 * Pitch   - the amount of rotation about the x-axis
 * Roll    - the amount of rotation about the y-axis
 */
class USML_DECLSPEC orientation {

public:

    /**
     * Default constructor
     */
    orientation() ;

    /**
     * Constructor using a given heading, pitch, and roll
     *
     * @param heading  rotation about the z-axis (deg)
     * @param pitch    rotation about the x-axis (deg)
     * @param roll     rotation about the y-axis (deg)
     */
    orientation( double heading, double pitch, double roll ) ;

    /**
     * Constructor using a tile angle/direction????
     */
    orientation( double angle, double direction ) ;

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
     * @return  current value stored in _pitch (deg)
     */
    double pitch() const {
        return -(_pitch*M_PI/180.0) ;
    }

    /**
     * Updates the pitch for the rotated system.
     * @param p     new pitch of the rotated system (deg)
     */
    void pitch( double p ) {
        _pitch = -p*M_PI/180.0 ;
    }

    /**
     * Current heading of the rotated system.
     * @return  current value stored in _heading (deg)
     */
    double heading() {
        return -(_heading*M_PI/180.0) ;
    }

    /**
     * Updates the heading for the rotated system.
     * @param h     new heading of the rotated system (deg)
     */
    void heading( double h ) {
        _heading = -h*M_PI/180.0 ;
    }

    /**
     * Current roll of the rotated system.
     * @return  current value stored in _roll (deg)
     */
    double roll() const {
        return (_roll*M_PI/180.0) ;
    }

    /**
     * Updates the roll for the rotated system.
     * @param r     new roll of the rotated system (deg)
     */
    void roll( double r ) {
        _roll = r*M_PI/180.0 ;
    }

    /**
     * Update all three rotation angles
     *
     * @param h     new heading of the rotated system (deg)
     * @param p     new pitch of the rotated system (deg)
     * @param r     new roll of the rotated system (deg)
     */
    void update_orientation( double h, double p, double r ) {
        _heading = -h*M_PI/180.0 ;
        _pitch = -p*M_PI/180.0 ;
        _roll = r*M_PI/180.0 ;
    }

private:

    /**
     * Member variables
     */
    double _heading ;
    double _pitch ;
    double _roll ;

    /**
     * Rotation vector
     */
    c_vector<double,3> _v ;
    c_vector<double,3> _v_cart ;

    /**
     * Convert from Spherical coordinates to Cartesian coordinates
     * This is done prior to applying the inverse rotation matrix
     * DEPRICATED: keeping this in the event that we wish to pass
     * the reference axis vector in as a spherical vector rather
     * than a Cartesian vector.
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
