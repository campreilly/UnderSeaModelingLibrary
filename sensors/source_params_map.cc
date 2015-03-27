/**
 * @file source_params_map.cc
 * Singleton map of source parameters.
 */
#include "source_params_map.h"

using namespace usml::sensors;

/**
 * Initialization of private static member _instance
 */
unique_ptr<source_params_map> source_params_map::_instance ;

/**
 * The mutex for the singleton pointer.
 */
read_write_lock source_params_map::_instance_mutex;

/**
 * Singleton Constructor - Double Check Locking Pattern DCLP
 */
source_params_map* source_params_map::instance() {
	source_params_map* tmp = _instance.get();
	if (tmp == NULL) {
		write_lock_guard guard(_instance_mutex);
		tmp = _instance.get();
		if (tmp == NULL) {
			tmp = new source_params_map();
			_instance.reset(tmp) ;
		}
	}
	return tmp;
}
