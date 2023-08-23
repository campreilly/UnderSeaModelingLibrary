/**
 * @file orientation.h
 * Physical orientation of an object in terms of the aircraft principal axes.
 */
#pragma once

#include <usml/ublas/ublas.h>

#include <boost/numeric/ublas/matrix_proxy.hpp>

namespace usml {
namespace types {

using namespace usml::ublas;

/// @ingroup bvector
/// @{

/**
 * Physical orientation of an object in the form of Tait–Bryan rotations.
 * The sign on the terms in the rotation matrix and the order in which
 * the rotations are applied have been adjusted to support an orientation
 * in terms of the forward, right, and up axes of the body being rotated.
 *
 * -  Yaw moves the front of the object from side to side.
 *    A positive yaw angle moves the nose to the right.
 *    A yaw value of zero points the object north.
 * -  Pitch moves the front of the object up and down.
 *    A positive pitch angle raises the front and lowers the back.
 *    A yaw value of zero leaves the object parallel to
 *    the surface of the earth.
 * -  Roll rotates the object around the longitudinal axis,
 *    the axis from back to front. A positive roll angle lifts
 *    the left side and lowers the right side of the object.
 *    In aircraft, a positive roll represents a bank to the right.
 * - The first column of the roation matrix represents direction of the body’s
 *    forward vector in the world coordinate system.   Similarly, the second
 *    column represents the right direction and the third column
 *    represents the up direction.
 *
 * When an orientation is formed directly from a rotation matrix, this
 * implementation delays construction of yaw, pitch, roll until they are
 * actually used, in an effort to make rotation multiplies faster.
 *
 * @xref Wikipedia, Davenport chained rotations,
 *       https://en.wikipedia.org/wiki/Davenport_chained_rotations#Tait-Bryan_chained_rotations
 */
class USML_DECLSPEC orientation {
   private:
    /** Compass heading. (deg) */
    double _yaw;

    /** Up/down tilt relative to local ocean surface. (deg) */
    double _pitch;

    /** Left/right tilt relative to local ocean surface. (deg) */
    double _roll;

    /** Rotation matrix for this orientation. */
    c_matrix<double, 3, 3> _rotation;

   public:
    /**
     * Default orientation is level object pointing north.
     */
    orientation() { update_orientation(0.0, 0.0, 0.0); }

    /**
     * Constructor using a given yaw, pitch, and roll.
     *
     * @param yaw   Compass heading. (deg)
     * @param pitch	Up/down tilt relative to local ocean surface. (deg)
     * @param roll  Left/right tilt relative to local ocean surface. (deg)
     */
    orientation(double yaw, double pitch, double roll) {
        update_orientation(yaw, pitch, roll);
    }

    /**
     * Construct orientation from rotation matrix.
     * Delays construction of yaw, pitch, roll until actually used.
     *
     * @param rotation	Orientation to copy.
     */
    orientation(matrix<double> rotation) {
        _rotation = rotation;
        _yaw = _pitch = _roll = NAN;
    }

    /**
     * Copy an existing orientation.
     * Delays construction of yaw, pitch, roll until actually used.
     *
     * @param o Orientation to copy.
     */
    orientation& operator=(const orientation& o) {
        _rotation = o._rotation;
        _yaw = _pitch = _roll = NAN;
        return *this;
    }

    /**
     * Copy an existing orientation.
     * Delays construction of yaw, pitch, roll until actually used.
     *
     * @param o Orientation to copy.
     */
    orientation(const orientation& o) {
        _rotation = o._rotation;
        _yaw = _pitch = _roll = NAN;
    }

    /**
     * Combine the effects of two rotations on orientation.
     * Delays construction of yaw, pitch, roll until actually used.
     *
     * @param parent 	Orientation of host object.
     * @param child 	Relative orientation of object mounted on host.
     */
    void rotate(const orientation& parent, const orientation& child) {
        _rotation = prod(parent._rotation, child._rotation);
        _yaw = _pitch = _roll = NAN;
    }

    /**
     * Apply this orientation to a set of vectors.
     *
     * Assumes that the orientation describes the rotation of a child body
     * on a parent body, the vector is in the coordinate system of the parent
     * body, and the desired vector is in the coordinate system of the child
     * body.
     *
     * @param north_east_up	A 3xN matrix of vectors in 3-space of the
     * 'parent' which will be converted to vectors of the 'child'.
     */
    void rotate_vector(matrix<double>& north_east_up) const {
        matrix<double> temp = trans(_rotation);
        for (size_t n = 0; n < north_east_up.size2(); n++) {
            matrix_column<matrix<double> > mc(north_east_up, n);
            mc = prod(temp, mc);
        }
    }

    /**
     * Current yaw of the rotated system.
     * @return Compass heading. (deg)
     */
    double yaw() {
        compute_angles();
        return _yaw;
    }

    /**
     * Updates yaw of the rotated system.
     * @param y Compass heading. (deg)
     */
    void yaw(double y) {
        _yaw = y;
        apply_rotation();
    }

    /**
     * Current yaw of the rotated system.
     * @return Up/down tilt relative to local ocean surface. (deg)
     */
    double pitch() {
        compute_angles();
        return _pitch;
    }

    /**
     * Updates pitch for the rotated system.
     * @param p Up/down tilt relative to local ocean surface. (deg)
     */
    void pitch(double p) {
        _pitch = p;
        apply_rotation();
    }

    /**
     * Current roll of the rotated system.
     * @return  Left/right tilt relative to local ocean surface. (deg)
     */
    double roll() {
        compute_angles();
        return _roll;
    }

    /**
     * Updates roll for the rotated system.
     * @param r Left/right tilt relative to local ocean surface. (deg)
     */
    void roll(double r) {
        _roll = r;
        apply_rotation();
    }

    /**
     * Current value of rotation matrix.
     * @return Copy of rotation matrix.
     */
    matrix<double> rotation() const { return _rotation; }

    /**
     * Direction toward the front of the object. North along the earth's
     * surface if object yaw and pitch are zero.
     *
     * @return Copy of direction vector.
     */
    vector<double> front() const {
        return boost::numeric::ublas::column(_rotation, 0);
    }

    /**
     * Direction toward the right side of the object. East along the earth's
     * surface if object yaw and roll are zero.
     *
     * @return Copy of direction vector.
     */
    vector<double> right() const {
        return boost::numeric::ublas::column(_rotation, 1);
    }

    /**
     * Direction toward the top of the object. Up from the earth's surface if
     * object pitch and roll are zero.
     *
     * @return Copy of direction vector.
     */
    vector<double> up() const {
        return boost::numeric::ublas::column(_rotation, 2);
    }

    /**
     * Update all three rotation angles
     *
     * @param yaw   Compass heading. (deg)
     * @param pitch	Up/down tilt relative to local ocean surface. (deg)
     * @param roll  Left/right tilt relative to local ocean surface. (deg)
     */
    void update_orientation(double yaw, double pitch, double roll) {
        _yaw = yaw;
        _pitch = pitch;
        _roll = roll;
        apply_rotation();
    }

   private:
    /**
     * Compute rotation matrix for current values of yaw, pitch, roll.
     */
    void apply_rotation();

    /**
     * Convert rotation to yaw, pitch, roll.
     */
    void compute_angles();
};

/// @}
}  // namespace types
}  // end of namespace usml
