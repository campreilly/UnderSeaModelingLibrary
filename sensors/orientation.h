/**
 * @file orientation.h
 */
#pragma once

#include <boost/numeric/ublas/vector.hpp>
#include <usml/usml_config.h>

using namespace boost::numeric::ublas ;

namespace usml {
namespace sensors {

/// @ingroup sensors
/// @{

/**
 * This class holds the physical orientation of the
 * sensor and provides a conversion from incident ray
 * coordinates to the spherical array coordinates.
 *
 * Uses definitions of heading, pitch, and roll based on 
 * aircraft principal axes:
 *
 * - Heading moves the front of the sensor from side to side. 
 *   A positive yaw angle moves the nose to the right.
 *   A heading value of zero points the sensor north.
 * - Pitch moves the front of the sensor up and down. 
 *   A positive pitch angle raises the front and lowers the back.
 *   A heading value of zero leaves the sensor parallel to
 *   the surface of the earth.
 * - Pitch rotates the sensor around the longitudinal axis, 
 *   the axis from back to front.  A positive roll angle lifts 
 *   the left side and lowers the right side of the sensor.
 *
 * @xref Wikipedia, Aircraft principal axes, 
 *       http://en.wikipedia.org/wiki/Aircraft_principal_axes
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
     * @param heading   rotation about the z-axis (deg)
     * @param pitch     rotation about the x-axis (deg)
     * @param roll      rotation about the y-axis (deg)
     * @param ref_axis  reference axis for spatial rotations (Cartesian)
     */
    orientation( double heading,
                 double pitch,
                 double roll,
                 vector<double> ref_axis ) ;

    /**
     * Constructor using a tile angle/direction. Using these values and holding
     * _heading to zero, we can compute a unique value of _pitch and _roll.
     *
     * @param angle     tilt from the nominal vertical axis,
     *                  equivalent to theta in spherical coordinates.
     * @param direction direction of the tilt from the nominal vertical axis,
     *                  equivalent to phi in spherical coordinates.
     */
    orientation( double angle, double direction ) ;

    /**
     * Destructor
     */
    virtual ~orientation() ;

    /**
     * Transforms a DE and AZ into a rotated equivalent in the rotated system.
     * This is used when a system is asymmetric and needs to be called everytime
     * a DE/AZ pair needs to be rotated.
     *
     * @param de        incident DE angle (rad)
     * @param az        incident AZ angle (rad)
     * @param de_prime  rotated DE angle (rad)
     * @param az_prime  rotated AZ angle (rad)
     */
    void apply_rotation( double& de,
                         double& az,
                         double* de_prime,
                         double* az_prime ) ;

    /**
     * Returns the current theta offset for the rotated reference axis
     * @return  theta for the rotated reference axis
     */
    inline double theta() const {
        return _theta ;
    }

    /**
     * Returns the current phi offset for the rotated reference axis
     * @return  phi for the rotated reference axis
     */
    inline double phi() const {
        return _phi ;
    }

    /**
     * Current pitch of the rotated system.
     * @return  current value stored in _pitch (deg)
     */
    double pitch() const {
        return -_pitch*180.0/M_PI ;
    }

    /**
     * Updates the pitch for the rotated system.
     * @param p     new pitch of the rotated system (deg)
     */
    void pitch( double p ) {
        _pitch = -p*M_PI/180.0 ;
        apply_rotation() ;
    }

    /**
     * Current heading of the rotated system.
     * @return  current value stored in _heading (deg)
     */
    double heading() const {
        return -(_heading*180.0/M_PI) ;
    }

    /**
     * Updates the heading for the rotated system.
     * @param h     new heading of the rotated system (deg)
     */
    void heading( double h ) {
        _heading = -h*M_PI/180.0 ;
        apply_rotation() ;
    }

    /**
     * Current roll of the rotated system.
     * @return  current value stored in _roll (deg)
     */
    double roll() const {
        return (_roll*180.0/M_PI) ;
    }

    /**
     * Updates the roll for the rotated system.
     * @param r     new roll of the rotated system (deg)
     */
    void roll( double r ) {
        _roll = r*M_PI/180.0 ;
        apply_rotation() ;
    }

    /**
     * Update all three rotation angles
     *
     * @param h     new heading of the rotated system (deg)
     * @param p     new pitch of the rotated system (deg)
     * @param r     new roll of the rotated system (deg)
     */
    void update_orientation( double h, double p, double r ) ;

	/**
	* Updates the tilt angle and direction.
	*
	* @param angle     tilt from the nominal vertical axis,
	*                  equivalent to theta in spherical coordinates.
	* @param direction direction of the tilt from the nominal vertical axis,
	*                  equivalent to phi in spherical coordinates.
	*/
	void update_orientation(double angle, double direction);


protected:

    /**
     * Member variables
     */
    double _heading ;
    double _pitch ;
    double _roll ;
    double _theta ;
    double _phi ;

    /**
     * Reference axis and local computation variables
     */
    vector<double> _axis ;
    double _x ;
    double _y ;
    double _z ;

    /**
     * Computes the orientation components of pitch, heading, and
     * roll from a tilt angle and direction. The tilt angle corresponds
     * to the tilt from the nominal vertical axis and the direction
     * corresponds to the azimuthal direction the tilt would be applied
     * to. Note this is for vertical arrays, but can be applied to horizontal
     * arrays. Where the tilt angle is almost nearly 90 degrees and the
     * direction corresponds to the heading of the array.
     *
     * @param angle     tilt angle of the array from the vertical reference axis
     * @param direction azimuthal direction of the tilt angle
     */
    void compute_orientation(double angle, double direction) ;

    /**
     * Rotates the reference axis and computes the necessary theta and
     * phi offset in spherical coordinates. This computation is only valid
     * for spatial objects that are symmetric about the reference axis. In the
     * event that the object is asymmetric, the theta and phi do not appropriately
     * account for the local coordinate roll of the object.
     */
    virtual void apply_rotation() ;

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

/// @}
}   // end of namespace sensors
}   // end of namespace usml
