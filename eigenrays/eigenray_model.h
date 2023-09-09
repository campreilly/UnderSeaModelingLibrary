/**
 * @file eigenray_model.h
 * A single acoustic path between a source and target.
 */
#pragma once

#include <usml/types/seq_vector.h>

#include <boost/numeric/ublas/vector.hpp>
#include <list>
#include <memory>

namespace usml {
namespace eigenrays {

using namespace usml::types;

/// @ingroup eigenrays
/// @{

/**
 * A single acoustic path between a source and target.
 */
struct eigenray_model {
    /// Alias for shared const reference to one eigenray.
    typedef std::shared_ptr<const eigenray_model> csptr;

    /// Default constructor.
    eigenray_model() {}

    /// Copy constructor.
    eigenray_model(const eigenray_model& other) = default;

    /**
     * Time of arrival for this acoustic path (sec).
     */
    double travel_time{0.0};

    /**
     * Frequencies over which propagation was computed (Hz).
     */
    seq_vector::csptr frequencies;

    /**
     * Propagation loss as a function of frequency (dB,positive).
     */
    vector<double> intensity;

    /**
     * Phase change as a function of frequency (radians).
     */
    vector<double> phase;

    /**
     * Initial depression/elevation angle at the
     * source location (degrees, positive is up).
     */
    double source_de{0.0};

    /**
     * Initial azimuthal angle at the source location
     * (degrees, clockwise from true north).
     */
    double source_az{0.0};

    /**
     * Final depression/elevation angle at the
     * target location (degrees, positive is up).
     */
    double target_de{0.0};

    /**
     * Final azimuthal angle at the target location
     * (degrees, clockwise from true north).
     */
    double target_az{0.0};

    /**
     * Number of surface reflections encountered along this path.
     */
    int surface{0};

    /**
     * Number of bottom reflections encountered along this path.
     */
    int bottom{0};

    /**
     * Number of caustics encountered along this path.
     */
    int caustic{0};

    /**
     * Number of upper vertices encountered along this path.
     */
    int upper{0};

    /**
     * Number of lower vertices encountered along this path.
     */
    int lower{0};
};

/**
 * List of acoustic paths between a source and target.
 */
typedef std::list<eigenray_model::csptr> eigenray_list;

/// @}
}  // namespace eigenrays
}  // namespace usml
