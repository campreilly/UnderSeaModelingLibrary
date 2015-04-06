/**
 * @file sensor_pair_manager.cc
 * Manages the containers for all the sensor pair's in use by the USML.
 */
#include <usml/sensors/sensor_pair_manager.h>
#include <usml/sensors/sensor_manager.h>
#include <boost/foreach.hpp>

using namespace usml::sensors;

/**
 * Initialization of private static member _instance
 */
unique_ptr<sensor_pair_manager> sensor_pair_manager::_instance;

/**
 * The _mutex for the singleton pointer.
 */
read_write_lock sensor_pair_manager::_instance_mutex;

/**
 * Singleton Constructor - Double Check Locking Pattern DCLP
 */
sensor_pair_manager* sensor_pair_manager::instance() {
	sensor_pair_manager* tmp = _instance.get();
	if (tmp == NULL) {
		write_lock_guard guard(_instance_mutex);
		tmp = _instance.get();
		if (tmp == NULL) {
			tmp = new sensor_pair_manager();
			_instance.reset(tmp);
		}
	}
	return tmp;
}

///**
// * Gets the fathometers for the map of sensors provided
// */
//eigenray_collection* sensor_pair_manager::find_pairs(sensor_query_map sensors)
//{
//
//    sensor_query_map::iterator iter;
//
//    for ( iter = sensors.begin(); iter != sensors.end(); ++iter)
//    {
//
//
//    }
//    return NULL;
//}

/**
 * Gets the fathometers for the map of sensors provided
 */
eigenray_collection* sensor_pair_manager::get_fathometers(sensor_query_map sensors)
{

    sensor_query_map::iterator iter;

    for ( iter = sensors.begin(); iter != sensors.end(); ++iter)
    {
//       sensor_model::id_type sensorID = iter->first;
//       xmitRcvModeType mode = iter->second;

    }
    return NULL;
}

/**
 * Builds new sensor_pair objects in reaction to notification
 * that a sensor is being added.
 */
void sensor_pair_manager::add_sensor(sensor_model::reference& sensor) {
	write_lock_guard guard(_manager_mutex);
	#ifdef USML_DEBUG
		cout << "sensor_pair_manager: add sensor("
		<< sensor->sensorID() << ")" << endl;
	#endif

	// add sensorID to the lists of active sources and receivers

	switch (sensor->mode()) {
	case usml::sensors::SOURCE:
		_src_list.insert(sensor->sensorID());
		break;
	case usml::sensors::RECEIVER:
		_rcv_list.insert(sensor->sensorID());
		break;
	case usml::sensors::BOTH:
		_src_list.insert(sensor->sensorID());
		_rcv_list.insert(sensor->sensorID());
		break;
	default:
		break;
	}

	// add monostatic pair for sensors when mode=BOTH

	if (sensor->mode() == usml::sensors::BOTH) {
		add_monostatic_pair(sensor);

	// add multistatic pairs when multistatic is true

	} else {
		if (sensor->mode() == usml::sensors::SOURCE) {
			if (sensor->source()->multistatic()) {
				add_multistatic_source(sensor);
			}
		} else {
			if (sensor->receiver()->multistatic()) {
				add_multistatic_receiver(sensor);
			}
		}
	}
}

/**
 * Removes existing sensor_pair objects in reaction to notification
 * that a sensor is about to be deleted.
 */
void sensor_pair_manager::remove_sensor(sensor_model::reference& sensor) {
	write_lock_guard guard(_manager_mutex);
	#ifdef USML_DEBUG
		cout << "sensor_pair_manager: remove sensor("
		<< sensor->sensorID() << ")" << endl;
	#endif

	// remove sensorID to the lists of active sources and receivers

	switch (sensor->mode()) {
	case usml::sensors::SOURCE:
		_src_list.erase(sensor->sensorID());
		break;
	case usml::sensors::RECEIVER:
		_rcv_list.erase(sensor->sensorID());
		break;
	case usml::sensors::BOTH:
		_src_list.erase(sensor->sensorID());
		_rcv_list.erase(sensor->sensorID());
		break;
	default:
		break;
	}

	// remove monostatic pair for sensors when mode=BOTH

	if (sensor->mode() == usml::sensors::BOTH) {
		remove_monostatic_pair(sensor);

		// remove multistatic pairs when multistatic is true

	} else {
		if (sensor->mode() == usml::sensors::SOURCE) {
			if (sensor->source()->multistatic()) {
				remove_multistatic_source(sensor);
			}
		} else {
			if (sensor->receiver()->multistatic()) {
				remove_multistatic_receiver(sensor);
			}
		}
	}
}

/**
 * Utility to build a monostatic pair
 */
void sensor_pair_manager::add_monostatic_pair(sensor_model::reference& sensor) {
	sensor_model::id_type sourceID = sensor->sensorID();
	std::stringstream hash_key;
	hash_key << sourceID << "_" << sourceID;
	sensor_pair::reference pair(new sensor_pair(sensor, sensor));
	_map.insert(hash_key.str(), pair);
	sensor->add_sensor_listener(pair);
	#ifdef USML_DEBUG
		cout << "sensor_pair_manager: add sensor_pair("
		<< sourceID << "," << sourceID << ")" << endl;
	#endif
}

/**
 * Utility to build a multistatic pair from the source.
 */
void sensor_pair_manager::add_multistatic_source(sensor_model::reference& source) {
	sensor_model::id_type sourceID = source->sensorID();
	BOOST_FOREACH( sensor_model::id_type receiverID, _rcv_list ) {
		if ( sourceID != receiverID ) {
			sensor_model::reference receiver =
			sensor_manager::instance()->find(receiverID);
			if ( receiver.get() != NULL &&
					receiver->receiver()->multistatic() )
			{
				std::stringstream hash_key;
				hash_key << sourceID << "_" << receiverID;
				sensor_pair::reference pair(
						new sensor_pair(source,receiver) );
				_map.insert( hash_key.str(), pair );
				source->add_sensor_listener(pair);
				receiver->add_sensor_listener(pair);
				#ifdef USML_DEBUG
					cout << "sensor_pair_manager: add sensor_pair("
					<< sourceID << "," << receiverID << ")" << endl;
				#endif
			}
		}
	}
}

/**
 * Utility to build a multistatic pair from the receiver.
 */
void sensor_pair_manager::add_multistatic_receiver(
		sensor_model::reference& receiver) {
	sensor_model::id_type receiverID = receiver->sensorID();
	BOOST_FOREACH( sensor_model::id_type sourceID, _src_list ) {
		if ( sourceID != receiverID ) { // exclude monostatic case
			sensor_model::reference source =
			sensor_manager::instance()->find(sourceID);
			if ( source.get() != NULL &&
					source->source()->multistatic() )
			{
				std::stringstream hash_key;
				hash_key << sourceID << "_" << receiverID;
				sensor_pair::reference pair(
						new sensor_pair(source,receiver) );
				_map.insert( hash_key.str(), pair );
				source->add_sensor_listener(pair);
				receiver->add_sensor_listener(pair);
				#ifdef USML_DEBUG
					cout << "sensor_pair_manager: add sensor_pair("
					<< sourceID << "," << receiverID << ")" << endl;
				#endif
			}
		}
	}
}

/**
 * Utility to build a monostatic pair
 */
void sensor_pair_manager::remove_monostatic_pair(sensor_model::reference& sensor) {
	sensor_model::id_type sourceID = sensor->sensorID();
	std::stringstream hash_key;
	hash_key << sourceID << "_" << sourceID;
	sensor_pair::reference pair = _map.find(hash_key.str());
	if (pair.get() != NULL) {
		sensor->remove_sensor_listener(pair);
		_map.erase(hash_key.str());
		#ifdef USML_DEBUG
			cout << "sensor_pair_manager: remove sensor_pair("
				 << sourceID << "," << sourceID << ")" << endl;
		#endif
	}
}

/**
 * Utility to build a multistatic pair from the source.
 */
void sensor_pair_manager::remove_multistatic_source(sensor_model::reference& source) {
	sensor_model::id_type sourceID = source->sensorID();
	BOOST_FOREACH( sensor_model::id_type receiverID, _rcv_list ) {
		if ( sourceID != receiverID ) {
			sensor_model::reference receiver =
			sensor_manager::instance()->find(receiverID);
			if ( receiver.get() != NULL &&
					receiver->receiver()->multistatic() )
			{
				std::stringstream hash_key;
				hash_key << sourceID << "_" << receiverID;
				sensor_pair::reference pair = _map.find(hash_key.str());
				if ( pair.get() != NULL ) {
					source->remove_sensor_listener(pair);
					receiver->remove_sensor_listener(pair);
					_map.erase( hash_key.str() );
					#ifdef USML_DEBUG
						cout << "sensor_pair_manager: remove sensor_pair("
							 << sourceID << "," << receiverID << ")" << endl;
					#endif
				}
			}
		}
	}
}

/**
 * Utility to build a multistatic pair from the receiver.
 */
void sensor_pair_manager::remove_multistatic_receiver(
		sensor_model::reference& receiver) {
	sensor_model::id_type receiverID = receiver->sensorID();
	BOOST_FOREACH( sensor_model::id_type sourceID, _src_list ) {
		if ( sourceID != receiverID ) { // exclude monostatic case
			sensor_model::reference source =
			sensor_manager::instance()->find(sourceID);
			if ( source.get() != NULL &&
					source->source()->multistatic() )
			{
				std::stringstream hash_key;
				hash_key << sourceID << "_" << receiverID;
				sensor_pair::reference pair = _map.find(hash_key.str());
				if ( pair.get() != NULL ) {
					source->remove_sensor_listener(pair);
					receiver->remove_sensor_listener(pair);
					_map.erase( hash_key.str() );
					#ifdef USML_DEBUG
						cout << "sensor_pair_manager: remove sensor_pair("
							 << sourceID << "," << receiverID << ")" << endl;
					#endif
				}
			}
		}
	}
}
