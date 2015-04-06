/**
 * @file beam_pattern_map.cc
 * Singleton map of beam pattern parameters.
 */
#include <usml/sensors/beam_pattern_map.h>

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
			// initialize new map
			tmp = new beam_pattern_map();

			// insert omni beam pattern as id #0
			beam_pattern_model* beam_patt = new beam_pattern_omni();
			beam_pattern_model::id_type id = 0 ;
			beam_patt->beamID(id) ;
			beam_pattern_model::reference beam_ref(beam_patt);
			tmp->insert(id, beam_ref);

			// initialize singleton
			_instance.reset(tmp) ;
		}
	}
	return tmp;
}
