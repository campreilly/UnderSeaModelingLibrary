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

			// insert omni beam pattern as OMNI, id #0
			beam_pattern_model::reference beam_ref(new beam_pattern_omni());
			tmp->insert(beam_ref.get()->beamID(), beam_ref);

			// insert cosine beam pattern as COSINE, id #1
            beam_ref = beam_pattern_model::reference(new beam_pattern_cosine()) ;
            tmp->insert(beam_ref.get()->beamID(), beam_ref);

			// initialize singleton
			_instance.reset(tmp) ;
		}
	}
	return tmp;
}

/**
* Reset the unique beam_pattern_map pointer to empty.
*/
void beam_pattern_map::reset() {
    write_lock_guard(_instance_mutex);
    _instance.reset();
}
