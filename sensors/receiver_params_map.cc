/**
 * @file receiver_params_map.cc
 * Singleton map of receiver parameters.
 */
#include <usml/sensors/receiver_params_map.h>

using namespace usml::sensors;

/**
 * Initialization of private static member _instance
 */
unique_ptr<receiver_params_map> receiver_params_map::_instance ;

/**
 * The mutex for the singleton pointer.
 */
read_write_lock receiver_params_map::_instance_mutex;

/**
 * Singleton Constructor - Double Check Locking Pattern DCLP
 */
receiver_params_map* receiver_params_map::instance() {
	receiver_params_map* tmp = _instance.get();
	if (tmp == NULL) {
		write_lock_guard guard(_instance_mutex);
		tmp = _instance.get();
		if (tmp == NULL) {
			tmp = new receiver_params_map();
			_instance.reset(tmp) ;
		}
	}
	return tmp;
}
