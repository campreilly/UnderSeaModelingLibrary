/**
 * @file sensor_manager.cc
 * Container for all the sensor's in use by the USML.
 */
#include <usml/sensors/sensor_manager.h>

using namespace usml::sensors;

/**
 * Initialization of private static member _instance
 */
unique_ptr<sensor_manager> sensor_manager::_instance;

/**
 * The _mutex for the singleton sensor_manager.
 */
read_write_lock sensor_manager::_instance_mutex;

/**
 * Singleton Constructor - Double Check Locking Pattern DCLP
 */
sensor_manager* sensor_manager::instance() {
	sensor_manager* tmp = _instance.get();
	if (tmp == NULL) {
		write_lock_guard guard(_instance_mutex);
		tmp = _instance.get();
		if (tmp == NULL) {
			tmp = new sensor_manager();
			_instance.reset(tmp);
		}
	}
	return tmp;
}

/**
* Default destructor.
*/
sensor_manager::~sensor_manager()
{
    // Remove all sensor_model pointers from _map
    sensor_map_template<sensor_model::id_type, sensor_model*>::iterator iter;
    for ( iter = _map.begin(); iter != _map.end(); ++iter )
    {
        sensor_model* sensor = iter->second;
        #ifdef USML_DEBUG
            cout << " ~sensor_manager: deleting sensor " << sensor << endl;
        #endif
        delete sensor;
    }
}

/**
 * Reset the sensor_manager to empty.
 */
void sensor_manager::reset() {
    write_lock_guard(_instance_mutex);
    _instance.reset();
}

/**
 * Construct a new instance of a specific sensor type.
 */
bool sensor_manager::add_sensor(sensor_model::id_type sensorID,
		sensor_params::id_type paramsID, const std::string& description )
{
	write_lock_guard guard(_manager_mutex);
	if (find(sensorID) != NULL ) {
		return false;
	}
	#ifdef USML_DEBUG
	    cout << "******" << endl ;
		cout << "sensor_manager: add sensor(" << sensorID << ")" << endl ;
	#endif

	sensor_model* created = new sensor_model(sensorID, paramsID, description);
	_map.insert(sensorID, created);
	sensor_pair_manager::instance()->add_sensor(created) ;
	return true ;
}

/**
 * Removes an existing sensor instance by sensorID.
 */
bool sensor_manager::remove_sensor(sensor_model::id_type sensorID) {
	write_lock_guard guard(_manager_mutex);
	#ifdef USML_DEBUG
    	cout << "******" << endl ;
		cout << "sensor_manager: remove sensor(" << sensorID << ")" << endl ;
	#endif
	sensor_model* removed = _map.find(sensorID) ;
	if ( removed == NULL ) return false ;
	sensor_pair_manager::instance()->remove_sensor(removed) ;
    delete removed;
	return _map.erase(sensorID);
}

/**
 * Updates an existing sensor instance by sensorID.
 */
bool sensor_manager::update_sensor(const sensor_model::id_type sensorID,
		const wposition1& position, const orientation& orientation,
		bool force_update)
{
	write_lock_guard guard(_manager_mutex);
	#ifdef USML_DEBUG
		cout << "******" << endl ;
		cout << "sensor_manager: update sensor(" << sensorID << ")" << endl ;
	#endif
	sensor_model* current_sensor = find(sensorID);
	if (current_sensor != NULL ) {
		current_sensor->update_sensor(position, orientation, force_update);
		return true;
	}
	return false;
}
