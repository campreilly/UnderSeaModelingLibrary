/**
 * @file wavefront_listener.h
 * Abstract listener for eigenray and eigenverb changes to sensor.
 */

#pragma once

#include <usml/eigenrays/eigenray_collection.h>
#include <usml/eigenverbs/eigenverb_collection.h>
#include <usml/usml_config.h>

namespace usml {
namespace sensors {
class sensor_model;
}
}  // namespace usml

namespace usml {
namespace wavegen {

using namespace usml::eigenrays;
using namespace usml::eigenverbs;
using namespace usml::sensors;

/// @ingroup wavegen
/// @{

/**
 * Abstract listener for eigenray and eigenverb changes to sensor.
 */
class USML_DECLSPEC wavefront_listener {
   public:
    /// Virtual destructor
    virtual ~wavefront_listener() {}

    /**
     * Notify listener of new wavefront data (eigenrays and eigenverbs) have
     * been computed for a sensor.
     *
     * @param sensor 		Sensor model that generated this wavefront data.
     * @param eigenrays 	Shared pointer to a list of eigenrays computed.
     * @param eigenverbs 	Shared pointer to a list of eigenverbs computed.
     */
    virtual void update_wavefront_data(
        const sensor_model* sensor, eigenray_collection::csptr eigenrays,
        eigenverb_collection::csptr eigenverbs) = 0;
};

/// @}
}  // end of namespace wavegen
}  // end of namespace usml
