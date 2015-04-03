/**
 * @file sensor_orientation.cc
 * Individual world location in geodetic earth coordinates
 * (latitude, longitude, and altitude).
 */
#include <usml/sensors/sensor_orientation.h>

using namespace usml::sensors;

/**
 * Default constructor.
 */
sensor_orientation::sensor_orientation() {
	heading(0.0) ;
	pitch(0.0) ;
	roll(0.0) ;
}

/**
 * Construct from all three angles.
 */
sensor_orientation::sensor_orientation(double heading, double pitch, double roll) {
	this->heading(heading) ;
	this->pitch(pitch) ;
	this->roll(roll) ;
}

/**
 * Construct from tilt angles.
 * @todo using dummy values for prototyping.
 */
sensor_orientation::sensor_orientation(double tilt, double direction) {
	heading(0.0) ;
	pitch(0.0) ;
	roll(0.0) ;
}

/**
 * Rotate a pair of DE/AZ angles from world coordinates
 * (north, east, up) into the rotated coordinate system of the array.
 * @todo using dummy values for prototyping.
 */
void rotate( double world_de, double world_az, double& array_de, double& array_az ) {
	array_de = world_de ;
	array_az = world_az ;
}



