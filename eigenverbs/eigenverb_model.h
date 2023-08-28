/**
 * @file eigenverb_model.h
 * Combination of source and receiver eignverbs for a bistatic pair.
 */
#pragma once

#include <usml/types/seq_vector.h>
#include <usml/types/wposition1.h>

#include <boost/numeric/ublas/vector.hpp>
#include <cstddef>
#include <list>
#include <memory>

namespace usml {
namespace eigenverbs {

using namespace usml::types;

/// @ingroup eigenverbs
/// @{

/**
 * Combination of source and receiver eignverbs for a bistatic pair. The name is
 * taken from the fact that eigenverbs provide discreet components of the total
 * reverberation in the same way that eigenrays provide discreet components of
 * the total transmission loss.
 *
 * To avoid conversions during the reverberation envelope generation process,
 * the attributes for direction, grazing, source_de, and source_az are
 * all expressed in radians.  Similar fields in the eigenray structure are
 * represented in degrees.
 */
struct eigenverb_model {
    /// Alias for shared const reference to one eignverb.
    typedef std::shared_ptr<eigenverb_model> csptr;

    /**
     * Types of interface interactions that eigenverbs need to
     * keep track of.
     */
    typedef enum {
        BOTTOM = 0,
        SURFACE = 1,
        VOLUME_UPPER = 2,
        VOLUME_LOWER = 3
    } interface_type;

    /**
     * One way travel time for this path (sec).
     */
    double time;

    /**
     * Frequencies of the wavefront (Hz)
     */
    seq_vector::csptr frequencies;

    /**
     * Fraction of total source level that reaches the ensonfied patch
     * (linear units).  Computed as fraction of solid angle for this ray
     * at launch, times the boundary and attenuation losses along this path,
     * and divided by the sine of the grazing angle.  The power is a function
     * of frequency because the boundary and attenuation losses are
     * functions of frequency.
     */
    vector<double> power;

    /**
     * Length of the D/E projection of the Gaussian beam onto
     * the interface (meters).
     */
    double length;

    /**
     * Width of the AZ projection of the Gaussian beam onto
     * the interface (meters).
     */
    double width;

    /**
     * Location of impact with the interface.
     */
    wposition1 position;

    /**
     * Compass heading for the "length" axis (radians, clockwise from true
     * north).
     */
    double direction;

    /**
     * The grazing angle of this path at impact of the boundary
     * (radians, positive is up).
     */
    double grazing;

    /**
     * The sound speed at the point of impact (m/s).
     */
    double sound_speed;

    /**
     * Index number of the of the launch DE.
     * Allows reverberation model to easily group eigenverbs by launch D/E.
     */
    size_t de_index;

    /**
     * The index of the launch AZ.
     * Allows reverberation model to easily group eigenverbs by launch AZ.
     */
    size_t az_index;

    /**
     * The depression/elevation (DE) angle of this path at the time of launch
     * (radians, positive is up).
     */
    double source_de;

    /**
     * The azimuthal (AZ) angle of this path at the time of launch
     * (radians, clockwise from true north).
     */
    double source_az;

    /**
     * Number of interactions with the surface boundary.
     */
    int surface;

    /**
     * Number of interactions with the bottom boundary.
     */
    int bottom;

    /**
     * Number of caustics encountered along this path.
     */
    int caustic;

    /**
     * Number of upper vertices encountered along this path.
     */
    int upper;

    /**
     * Number of lower vertices encountered along this path.
     */
    int lower;
};

/*
 * List of Gaussian projections used for reverberation.
 */
typedef std::list<eigenverb_model::csptr> eigenverb_list;

/// @}
}  // end of namespace eigenverbs
}  // end of namespace usml
