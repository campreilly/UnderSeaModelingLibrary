/**
 * @file bvector.h
 * Vector relative to body along the aircraft principal axes.
 */
#pragma once

#include <usml/ublas/ublas.h>
#include <usml/types/orientation.h>

namespace usml {
namespace types {

using namespace usml::ublas;

/// @ingroup bvector
/// @{

/**
 * Vector relative to body along the aircraft principal axes.
 */
class USML_DECLSPEC bvector {
private:

	/** Vector relative to body. */
	c_vector<double, 3> _vector;

public:

    /**
     * Constructor using empty parameters.
     */
    bvector() : _vector() {
    }

    /**
     * Constructor using a given front, right, and up.
     *
     * @param f     Offset toward the front of the object (meters).
     * @param r     Offset toward the right side of the object (meters).
     * @param u     Offset toward the top of the object (meters).
     */
    bvector(double f, double r, double u) {
        front(f);
        right(r);
        up(u);
    }

    /**
     * Constructor using a given DE and AZ angles
     *
     * @param DE    Depression/elevation angle (deg).
     * @param AZ    Azimuthal angle (deg).
     */
    bvector(double DE, double AZ) {
        const double de = to_radians(DE);
        const double az = to_radians(AZ);
        const double cos_de = cos(de);
        const double sin_de = sin(de);
        const double cos_az = cos(az);
        const double sin_az = sin(az);
        front( cos_de * cos_az );
        right( cos_de * sin_az );
        up( sin_de );
    }

	/**
	 * Construct from existing vector.
	 * Delays construction of front, right, up until actually used.
	 *
	 * @param v vector to copy.
	 */
	bvector(vector<double> v) {
		_vector = v ;
	}

	/**
     * Assign from existing bvector.
	 * Delays construction of front, right, up until actually used.
	 *
     * @param v vector to copy.
	 */
	bvector& operator=(const bvector& v) {
		_vector = v._vector;
		return *this ;
	}

	/**
     * Copy from existing bvector.
	 * Delays construction of front, right, up until actually used.
	 *
     * @param v vector to copy.
	 */
	bvector(const bvector& v) {
		_vector = v._vector;
	}

	/**
	 * Vector component toward the front of the object. North along the
	 * earth's surface if object yaw and pitch are zero.
	 *
	 * @return Vector component toward the front of the object (meters).
	 */
	double front() const {
		return _vector(0);
	}

	/**
	 * Updates vector component toward the front of the object.
	 *
	 * @param f Vector component toward the front of the object (meters).
	 */
	void front(double f) {
		_vector(0) = f;
	}

	/**
	 * Vector component toward the right side of the object. East along the
	 * earth's surface if object yaw and roll are zero.
	 *
	 * @return Vector component toward the right side of the object (meters).
	 */
	double right() const {
		return _vector(1);
	}

	/**
	 * Updates vector component toward the right side of the object.
	 * @param r Offset toward the right side of the object (meters).
	 */
	void right(double r) {
		_vector(1) = r;
	}

	/**
	 * Vector component toward the top of the object. Up from the
	 * earth's surface if object pitch and roll are zero.
	 *
	 * @return Vector component toward the top of the object (meters).
	 */
	double up() const {
		return _vector(2);
	}

	/**
	 * Updates vector component toward the top of the object.
	 * @param u Vector component toward the top of the object (meters).
	 */
	void up(double u) {
		_vector(2) = u;
	}

	/**
	 * Current value of vector relative to body.
	 * @return Copy of vector relative to body.
	 */
	vector<double> data() const {
		return _vector;
	}

	/**
	 * Combine the effects of two rotations on orientation.
	 * Delays construction of yaw, pitch, roll until actually used.
	 *
	 * @param parent 	Orientation of host object.
	 * @param child 	Relative orientation of object mounted on host.
	 */
	void rotate(const orientation& parent, const bvector& child) {
		_vector = prod(parent.rotation(), child._vector);
	}

};

/// @}
}
}
