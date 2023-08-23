/**
 * @file wavefront_listener.h
 * Abstract listener for eigenray and eigenverb changes to sensor.
 */

#pragma once

#include <usml/eigenrays/eigenray_collection.h>
#include <usml/eigenverbs/eigenverb_collection.h>
#include <usml/usml_config.h>

namespace usml {
namespace platforms {
class sensor_model;
}
}

namespace usml {
namespace wavegen {

using namespace usml::eigenrays;
using namespace usml::eigenverbs;
using namespace usml::platforms;

/// @ingroup wavegen
/// @{

/**
 * Abstract listener for eigenray and eigenverb changes to sensor.
 */
class USML_DECLSPEC wavefront_listener {
   public:
    /**
     * Notify listener of new wavefront data (eigenrays and eigenverbs) have
     * been computed for a sensor.
     *
     * @param sensor 		Sensor model that generated this wavefront data.
     * @param eigenrays 	Shared pointer to an eigenrays computed.
     * @param eigenverbs 	Shared pointer to an eigenverbs computed.
     */
    virtual void update_wavefront_data(
        const sensor_model* sensor, eigenray_collection::csptr eigenrays,
        eigenverb_collection::csptr eigenverbs) = 0;
};

/// @}
}  // end of namespace wavegen
}  // end of namespace usml
