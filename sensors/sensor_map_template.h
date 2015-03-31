/**
 * @file sensor_map_template.h
 * Base class for singleton maps of sensor type parameters.
 */
#pragma once

#include <usml/ublas/ublas.h>
#include <usml/threads/read_write_lock.h>
#include <usml/threads/smart_ptr.h>
#include <map>

namespace usml {
namespace sensors {

using namespace usml::ublas;
using namespace usml::threads;

/// @ingroup sensors
/// @{

/**
 * Base class for singleton maps of sensor type parameters.
 * Defined as a template that wraps a std::map and implements the erase,
 * find, and insert methods. The map_template was designed to contain a
 * std::map vs inheriting from a std::map because:
 *
 *   - General OOD principal to constrain the public API, std::map is wide open.
 *   - Limited API provides for easily extendible child classes.
 *   - Unwrapping std:map calls will all be internal, allowing for cleaner map_template calls.
 *
 * Assumes that the map is the storage container for sensor parameters.
 * Instead of storing pointers, developers must wrap them in a shared_ptr<>.
 * This causes them to be automatically cleaned up when the entry
 * is destroyed.  Use scoped_ptr<> for parameters that you intend to clone,
 * and shared_ptr<> if a single copy of the parameters are shared.
 *
 * @tparam	key_type		Key used to lookup the sensor type parameters.
 * @tparam	mapped_type		Sensor type parameters.
 */
template<class key_type, class mapped_type>
class USML_DLLEXPORT sensor_map_template {

public:
	/**
	 * Data type used for store beam patterns in this sensor.
	 */
	typedef std::map<key_type, mapped_type> container ;

	/**
	 * Finds the sensor parameters associated with the keyID.
	 *
	 * @param 	keyID 	Key used to lookup the sensor type parameters.
	 * @return			Sensor parameters if found, blank entry if not.
	 */
	mapped_type find(key_type keyID) const {
		read_lock_guard guard(_map_mutex);
		if (_map.count(keyID) == 0) return mapped_type();
		return _map.find(keyID)->second;
	}

	/**
	 * Inserts the supplied mapped_type into the map with the key provided.
	 * Ignores request if there is an existing entry for this key.
	 *
	 * @param 	keyID 	Key used to lookup the sensor type parameters.
	 * @param	mapped	Sensor type parameters.
	 * @return 			False if keyID was already in the map.
	 */
	bool insert(key_type keyID, mapped_type mapped) {
		write_lock_guard guard(_map_mutex);
		if (_map.count(keyID) != 0) return false;
		_map[keyID] = mapped;
		return true;
	}

	/**
	 * Erases the sensor parameters associated with the keyID.
	 *
	 * @param 	keyID 	Key used to lookup the sensor type parameters.
	 * @return 			False if keyID was not found in the map.
	 */
	bool erase(key_type keyID) {
		write_lock_guard guard(_map_mutex);
		if (_map.count(keyID) == 0) return false;
		_map.erase(keyID);
		return true;
	}

	/**
	 * Default constructor.
	 */
	sensor_map_template() {
	}

	/**
	 * Virtual destructor.
	 */
	virtual ~sensor_map_template() {
	}

private:

	/**
	 * Prevent access to copy constructor
	 */
	sensor_map_template(sensor_map_template&);

	/**
	 * Prevent access to assignment operator
	 */
	sensor_map_template& operator=(sensor_map_template&);

	/**
	 * The std::map that stores the mapped_types by key_type
	 */
	container _map;

	/**
	 * The _mutex for multi-threaded access.
	 */
	mutable read_write_lock _map_mutex;
};

/// @}
}// end of namespace sensors
} // end of namespace usml
