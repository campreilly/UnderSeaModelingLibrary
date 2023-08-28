/**
 * @file biverb_model.h
 * Combination of source and receiver eignverbs for a bistatic pair.
 */
#pragma once

#include <usml/types/seq_vector.h>

#include <boost/numeric/ublas/vector.hpp>
#include <cstddef>
#include <list>
#include <memory>

namespace usml {
namespace biverbs {

using namespace usml::types;

/// @ingroup biverbs
/// @{

/**
 * Combination of source and receiver eignverbs for a bistatic pair.
 * Pre-computes all of the geometry related elements of eigenverb overlap except
 * the application of the beam patterns. Assumes that the beam patterns many
 * change more quickly than the geometry of the source and receiver to each
 * scattering patch.
 *
 * To avoid conversions during the reverberation envelope generation process,
 * the attributes for direction, grazing, source_de, and source_az are
 * all expressed in radians.  Similar fields in the eigenray structure are
 * represented in degrees.
 */
struct biverb_model {
    /// Alias for shared const reference to biverb.
    typedef std::shared_ptr<biverb_model> csptr;

    /**
     * Two way travel time for this path (sec).
     */
    double time;

    /**
     * Frequencies of the wavefront (Hz)
     */
    seq_vector::csptr frequencies;

    /**
     * Combined power of the source eigenverb, receiver eigenverb, and
     * scattering strength of the interface as function of frequency.
     */
    vector<double> power;

    /**
     * Echo duration for this scattering patch as seen from the perspective of
     * the receiver.
     */
    double duration;

    /**
     * Index number of the of the launch DE at the receiver.
     * Allows reverberation model to easily group eigenverbs by launch D/E.
     */
    size_t de_index;

    /**
     * The index of the launch AZ at the receiver.
     * Allows reverberation model to easily group eigenverbs by launch AZ.
     */
    size_t az_index;

    /**
     * The depression/elevation (DE) angle of this path at the time of launch
     * from the source. (radians, positive is up).
     */
    double source_de;

    /**
     * The azimuthal (AZ) angle of this path at the time of launch from the
     * source. (radians, clockwise from true north).
     */
    double source_az;

    /**
     * The depression/elevation (DE) angle of this path at the time of launch
     * from the receiver. (radians, positive is up).
     */
    double receiver_de;

    /**
     * The azimuthal (AZ) angle of this path at the time of launch from the
     * receiver (radians, clockwise from true north).
     */
    double receiver_az;

    /**
     * Number of interactions with the surface boundary along source path.
     */
    int source_surface;

    /**
     * Number of interactions with the bottom boundary along source path.
     */
    int source_bottom;

    /**
     * Number of caustics encountered along source path.
     */
    int source_caustic;

    /**
     * Number of upper vertices encountered along source path.
     */
    int source_upper;

    /**
     * Number of lower vertices encountered along source path.
     */
    int source_lower;
};

/*
 * List of Gaussian projections used for reverberation.
 */
typedef std::list<biverb_model::csptr> biverb_list;

/// @}
}  // end of namespace biverbs
}  // end of namespace usml
