/**
 * @file receiver_params_map.h
 * Singleton map of receiver parameters.
 */
#pragma once

#include <usml/usml_config.h>
#include <usml/sensors/sensor_map_template.h>
#include <usml/sensors/receiver_params.h>
#include <usml/threads/read_write_lock.h>

namespace usml {
namespace sensors {

using namespace usml::threads;

/// @ingroup sensors
/// @{

/**
 * Singleton map of receiver parameters.  Each sensor makes a clone of these
 * parameters for its own use.
 */
class USML_DECLSPEC receiver_params_map: public sensor_map_template<
	receiver_params::id_type, receiver_params::const_reference >
{

public:

	/**
	 * Provides a reference to the receiver_params_map singleton.
	 * If this is the first time that this has been invoked, the singleton
	 * is automatically constructed.  The double check locking pattern
	 * is used to prevent multiple threads from simultaneously trying to
	 * construct the singleton.
	 *
	 * @xref 	Meyers, S., Alexandrescu, A.: C++ and the perils of
	 * 		 	double-checked locking. Dr. Dobbs Journal (July-August 2004)
	 * @return  Reference to the receiver_params_map singleton.
	 */
	static receiver_params_map* instance();

private:

	/**
	 * The singleton access pointer.
	 */
	static unique_ptr<receiver_params_map> _instance;

	/**
	 * The mutex for the singleton pointer.
	 */
	static read_write_lock _instance_mutex;
};

/// @}
}// end of namespace sensors
} // end of namespace usml
