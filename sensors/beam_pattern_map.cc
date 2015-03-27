/**
 * @file beam_pattern_map.cc
 * Singleton map of beam pattern parameters.
 */
#include "beam_pattern_map.h"

using namespace usml::sensors;

/**
 * Initialization of private static member _instance
 */
unique_ptr<beam_pattern_map> beam_pattern_map::_instance ;

/**
 * The mutex for the singleton pointer.
 */
read_write_lock beam_pattern_map::_instance_mutex;

/**
 * Singleton Constructor - Double Check Locking Pattern DCLP
 */
beam_pattern_map* beam_pattern_map::instance() {
	beam_pattern_map* tmp = _instance.get();
	if (tmp == NULL) {
		write_lock_guard guard(_instance_mutex);
		tmp = _instance.get();
		if (tmp == NULL) {
			tmp = new beam_pattern_map();
			_instance.reset(tmp) ;
		}
	}
	return tmp;
}
