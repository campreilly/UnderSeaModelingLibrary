/**
 * @file beam_pattern_map.h
 * Singleton map of beam pattern parameters.
 */
#pragma once

#include <cstddef>

#include <usml/usml_config.h>

#include <usml/sensors/beams.h>
#include <usml/sensors/beam_pattern_model.h>
#include <usml/sensors/sensor_map_template.h>
#include <usml/threads/read_write_lock.h>

namespace usml {
namespace sensors {

using namespace usml::threads;

/// @ingroup sensors
/// @{

/**
 * Singleton map of beam pattern parameters.  Stores the beam patterns
 * as shared pointers so that multiple sensors can reference the same
 * beam pattern.
 *
 * During construction, the map automatically inserts a beam_pattern_omni
 * instance as the entry for beamID #0.
 */
class USML_DECLSPEC beam_pattern_map: public sensor_map_template<
        beam_pattern_model::id_type, beam_pattern_model::reference>
{
public:

    /**
     * Provides a reference to the beam_pattern_map singleton.
     * If this is the first time that this has been invoked, the singleton
     * is automatically constructed.  The double check locking pattern
     * is used to prevent multiple threads from simultaneously trying to
     * construct the singleton.
     *
     * @xref     Meyers, S., Alexandrescu, A.: C++ and the perils of
     *              double-checked locking. Dr. Dobbs Journal (July-August 2004)
     * @return  Reference to the beam_pattern_map singleton.
     */
    static beam_pattern_map* instance() ;

    /**
     * Reset the unique beam_pattern_map pointer to empty.
     */
    static void reset();

private:

    /**
     * The singleton access pointer.
     */
    static unique_ptr<beam_pattern_map> _instance;

    /**
     * The mutex for the singleton pointer.
     */
    static read_write_lock _instance_mutex ;
};

/// @}
} // end of namespace sensors
} // end of namespace usml
