/**
 * @file sensor_orientation.h
 * Rotation of beam pattern maximum response axis (MRA) relative to
 * north, east, up directions.
 */
#pragma once

#include <usml/ublas/ublas.h>

namespace usml {
namespace sensors {

using namespace usml::ublas;

/// @ingroup sensors
/// @{

/**
 * Rotation of beam pattern maximum response axis relative to
 * north, east, up direction.
 */
class USML_DECLSPEC sensor_orientation {

public:

	/**
	 * Default constructor.
	 */
	sensor_orientation();

	/**
	 * Construct from all three angles.
	 *
	 * @param  heading	Compass heading of MRA (degrees).
	 * @param  pitch    MRA tilt along north-south direction (degrees).
	 * @param  roll     Initial altitude (meters).
	 */
	sensor_orientation(double heading, double pitch, double roll);

	/**
	 * Construct from tilt angles.  Tilt angle is the angle of the array
	 * from its nominal vertical position. Tilt direction is the direction
	 * of the tilt as the azimuthal direction of the upper part of the
	 * array relative to the lower part.  Note that, while these tilt
	 * parameters are selected primarily for suspended arrays with a nominal
	 * vertical orientation, they are also applicable to towed line arrays
	 * where the tilt angle would be very nearly 90 degrees and the tilt
	 * direction would correspond to the direction in which it was being towed.
	 *
	 * @param  tilt			Tilt angle (degrees).
	 * @param  direction    Tilt direction (degrees).
	 */
	sensor_orientation(double tilt, double direction);

public:

	/**
	 * Compass heading of MRA (degrees).
	 * Clockwise rotation around "up" direction.
	 */
	double heading() const {
		return to_degrees(euler(0));
	}

	/**
	 * Compass heading of MRA (degrees).
	 * Clockwise rotation around "up" direction.
	 */
	void heading(double heading) {
		euler(0) = to_radians(heading);
	}

	/**
	 * MRA tilt along north-south direction (degrees).
	 * Counter-clockwise rotation around "east" direction.
	 */
	double pitch() const {
		return to_degrees(euler(1));
	}

	/**
	 * MRA tilt along north-south direction (degrees).
	 * Counter-clockwise rotation around "east" direction.
	 */
	void pitch(double pitch) {
		euler(1) = to_radians(pitch);
	}

	/**
	 * MRA tilt along east-west direction (degrees).
	 * Counter-clockwise rotation around "north" direction.
	 */
	double roll() const {
		return to_degrees(euler(2));
	}

	/**
	 * MRA tilt along east-west direction (degrees).
	 * Counter-clockwise rotation around "north" direction.
	 */
	void roll(double roll) {
		euler(1) = to_radians(roll);
	}

	/**
	 * Rotate a pair of DE/AZ angles from world coordinates
	 * (north, east, up) into the rotated coordinate system of the array.
	 */
	void rotate(double world_de, double world_az, double& array_de, double& array_az);

private:

	/**
	 * Euler angles in radians.
	 */
	c_vector<double, 3> euler;
};

/// @}
}// end of ocean namespace
} // end of sensors namespace
