/**
 * @file orientation.cc
 * Physical orientation of an object in terms of the aircraft principal axes.
 */
#include <usml/types/orientation.h>

#include <boost/numeric/ublas/assignment.hpp>
#include <boost/numeric/ublas/io.hpp>

using namespace usml::types;

/**
 * Compute rotation matrix for current orientation.
 */
void orientation::apply_rotation() {
    // compute rotation matrix for roll around x-axis (=north=forward)

    const double roll = to_radians(_roll);
    const double cosr = cos(roll);
    const double sinr = sin(roll);
    c_matrix<double, 3, 3> Rroll;
    Rroll <<= 1.0, 0.0, 0.0, 0.0, cosr, sinr, 0.0, -sinr, cosr;

    // compute rotation matrix for pitch around y-axis (=east=right)

    const double pitch = to_radians(_pitch);
    const double cosp = cos(pitch);
    const double sinp = sin(pitch);
    c_matrix<double, 3, 3> Rpitch;
    Rpitch <<= cosp, 0.0, -sinp, 0.0, 1.0, 0.0, sinp, 0.0, cosp;

    // compute rotation matrix for yaw around z-axis (=up)

    const double yaw = to_radians(_yaw);
    const double cosy = cos(yaw);
    const double siny = sin(yaw);
    c_matrix<double, 3, 3> Ryaw;
    Ryaw <<= cosy, -siny, 0.0, siny, cosy, 0.0, 0.0, 0.0, 1.0;

    // concatenate rotations using Tait-Bryan conventions

    c_matrix<double, 3, 3> temp = prod(Rpitch, Rroll);
    _rotation = prod(Ryaw, temp);
}

/**
 * Delayed construction for yaw, pitch, roll.
 */
void orientation::compute_angles() {
    if (std::isnan(_yaw)) {
        _yaw = -to_degrees(-atan2(_rotation(1, 0), _rotation(0, 0)));
    }
    if (std::isnan(_pitch)) {
        _pitch = to_degrees(asin(_rotation(2, 0)));
    }
    if (std::isnan(_roll)) {
        _roll = -to_degrees(atan2(_rotation(2, 1), _rotation(2, 2)));
    }
}
