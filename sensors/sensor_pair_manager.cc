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
 * The _mutex for the singleton sensor_pair_manager.
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

/**
 * Default destructor.
 */
sensor_pair_manager::~sensor_pair_manager() {

    // Remove all sensor_pair pointers from the _map
    sensor_map_template<std::string, sensor_pair*>::iterator iter;
    for ( iter = _map.begin(); iter != _map.end(); ++iter )
    {
        sensor_pair* pair_data = iter->second;
        delete pair_data;
    }
}

/**
 * Reset the sensor_pair_manager instance to empty.
 */
void sensor_pair_manager::reset() {
    write_lock_guard(_instance_mutex);
    _instance.reset();
}

/**
 * Finds all the keys in the _map that are in the sensor_query_map
 */
std::set<std::string> sensor_pair_manager::find_pairs(sensor_query_map& sensors)
{   
    std::set<std::string> hash_keys;
    std::set<sensor_model::id_type> source_ids;
    std::set<sensor_model::id_type> receiver_ids;
    std::set<sensor_model::id_type>::iterator test_iter;

    // Create a source_keys list and a receiver_key list 
    // of the requested items
    xmitRcvModeType mode;
    sensor_model::id_type sensorID;
    std::pair<sensor_model::id_type, xmitRcvModeType> p;
    BOOST_FOREACH(p, sensors)
    {
        sensorID = p.first;
        mode = p.second;
      
        // Only add keys if the sensorID already exist in its respected list
        switch ( mode )
        {
            case usml::sensors::SOURCE:
                test_iter = _src_list.find(sensorID);
                if ( test_iter != _src_list.end() ) {
                    source_ids.insert(sensorID);
                }
                break;
            case usml::sensors::RECEIVER:
                test_iter = _rcv_list.find(sensorID);
                if ( test_iter != _rcv_list.end() ) {
                    receiver_ids.insert(sensorID);
                }
                break;
            case usml::sensors::BOTH:
                test_iter = _src_list.find(sensorID);
                if ( test_iter != _src_list.end() )
                {
                    source_ids.insert(sensorID);
               
                    test_iter = _rcv_list.find(sensorID);
                    if ( test_iter != _rcv_list.end() ) {
                        receiver_ids.insert(sensorID);
                    } else { // Did not exist, backout of source_ids
                        source_ids.erase(sensorID);
                    }
                }
                break;
            default:
                break;
        }
    }

    // Build hash_keys from source_ids and receiver_ids 
    BOOST_FOREACH(sensor_model::id_type srcID, source_ids)
    {
        BOOST_FOREACH(sensor_model::id_type rcvID, receiver_ids)
        {
            std::string key = generate_hash_key(srcID, rcvID);
            hash_keys.insert(key);
        }
    }
    
    return hash_keys;
}

/**
 * Gets the fathometers for the query of sensors provided
 */
fathometer_model::fathometer_package sensor_pair_manager::get_fathometers(sensor_query_map sensors)
{
    sensor_pair* pair;
    read_lock_guard guard(_manager_mutex);

    std::set<std::string> keys = find_pairs(sensors);
    fathometer_model::fathometer_package fathometers;
    fathometers.reserve(keys.size());
    BOOST_FOREACH(std::string s, keys)
    {
        pair = _map.find(s);
        sensor_pair* pair_data = pair;
        if ( pair_data != NULL ) {
            fathometer_model::reference fathometer = pair_data->fathometer();
            if ( fathometer.get() != NULL ) {
                fathometers.push_back(fathometer.get());
            }
        }
    }
    return fathometers;
}

/**
 * Writes the fathometers provided
 */
void sensor_pair_manager::write_fathometers(fathometer_model::fathometer_package fathometers, const char* filename)
{
    NcFile* nc_file = new NcFile(filename, NcFile::Replace);
    nc_file->add_att("Conventions", "COARDS");

    // dimensions

    NcDim *fathometer_dim = nc_file->add_dim("fathometers", ( long ) fathometers.size());
    NcVar *fathometer_index_var = nc_file->add_var("fathometer_index", ncLong, fathometer_dim);

    // fathometer_model attributes

    NcVar *source_id = nc_file->add_var("source_id", ncInt);
    NcVar *receiver_id = nc_file->add_var("receiver_id", ncInt);
    NcVar *slant_range = nc_file->add_var("slant_range", ncDouble);
    NcVar *distance_from_sensor = nc_file->add_var("distance_from_sensor", ncDouble);
    NcVar *depth_offset = nc_file->add_var("depth_offset", ncDouble);

    // coordinates

    NcVar *src_lat_var = nc_file->add_var("source_latitude", ncDouble);
    NcVar *src_lng_var = nc_file->add_var("source_longitude", ncDouble);
    NcVar *src_alt_var = nc_file->add_var("source_altitude", ncDouble);

    NcVar *rcv_lat_var = nc_file->add_var("receiver_latitude", ncDouble);
    NcVar *rcv_lng_var = nc_file->add_var("receiver_longitude", ncDouble);
    NcVar *rcv_alt_var = nc_file->add_var("receiver_altitude", ncDouble);

    // units

    fathometer_index_var->add_att("units", "count");

    src_lat_var->add_att("units", "degrees_north");
    src_lng_var->add_att("units", "degrees_east");
    src_alt_var->add_att("units", "meters");
    src_alt_var->add_att("positive", "up");
    
    rcv_lat_var->add_att("units", "degrees_north");
    rcv_lng_var->add_att("units", "degrees_east");
    rcv_alt_var->add_att("units", "meters");
    rcv_alt_var->add_att("positive", "up");
    
    int item;
    double v;
    int index = 0; // current index number
  
    BOOST_FOREACH(fathometer_model* fathometer, fathometers)
    {    
        // wite base attributes

        fathometer_index_var->set_cur(index);
        fathometer_index_var->put(&index, 1);
        item = fathometer->source_id(); source_id->put(&item);
        item = fathometer->receiver_id(); receiver_id->put(&item);

        v = fathometer->slant_range();  slant_range->put(&v);
        v = fathometer->distance_from_sensor();  distance_from_sensor->put(&v);
        v = fathometer->depth_offset();  depth_offset->put(&v);
        
        // write source parameters

        v = fathometer->source_position().latitude();    src_lat_var->put(&v);
        v = fathometer->source_position().longitude();   src_lng_var->put(&v);
        v = fathometer->source_position().altitude();    src_alt_var->put(&v);

        // write receiver parameters

        v = fathometer->receiver_position().latitude();    rcv_lat_var->put(&v);
        v = fathometer->receiver_position().longitude();   rcv_lng_var->put(&v);
        v = fathometer->receiver_position().altitude();    rcv_alt_var->put(&v);
        

        // Get the eigenray list for current fathometer
        eigenray_list eigenrays = fathometer->eigenrays();
       
        long num_eigenrays = ( long ) eigenrays.size();
        long num_frequencies = ( long ) eigenrays.begin()->frequencies->size();

        // dimensions
        NcDim *eigenray_dim = nc_file->add_dim("eigenrays", num_eigenrays);
        NcVar *eigenray_num_var = nc_file->add_var("eigenray_num", ncLong, eigenray_dim);
        NcDim *freq_dim = nc_file->add_dim("frequency", num_frequencies);
        NcVar *freq_var = nc_file->add_var("frequency", ncDouble, freq_dim);

        NcVar *intensity_var = nc_file->add_var("intensity", ncDouble, eigenray_dim, freq_dim);
        NcVar *phase_var = nc_file->add_var("phase", ncDouble, eigenray_dim, freq_dim);
        NcVar *time_var = nc_file->add_var("travel_time", ncDouble, eigenray_dim);
        NcVar *source_de_var = nc_file->add_var("source_de", ncDouble, eigenray_dim);
        NcVar *source_az_var = nc_file->add_var("source_az", ncDouble, eigenray_dim);
        NcVar *target_de_var = nc_file->add_var("target_de", ncDouble, eigenray_dim);
        NcVar *target_az_var = nc_file->add_var("target_az", ncDouble, eigenray_dim);
        NcVar *surface_var = nc_file->add_var("surface", ncShort, eigenray_dim);
        NcVar *bottom_var = nc_file->add_var("bottom", ncShort, eigenray_dim);
        NcVar *caustic_var = nc_file->add_var("caustic", ncShort, eigenray_dim);

        eigenray_num_var->add_att("units", "count");

        intensity_var->add_att("units", "dB");
        phase_var->add_att("units", "radians");
        time_var->add_att("units", "seconds");

        source_de_var->add_att("units", "degrees");
        source_de_var->add_att("positive", "up");
        source_az_var->add_att("units", "degrees_true");
        source_az_var->add_att("positive", "clockwise");

        target_de_var->add_att("units", "degrees");
        target_de_var->add_att("positive", "up");
        target_az_var->add_att("units", "degrees_true");
        target_az_var->add_att("positive", "clockwise");

        surface_var->add_att("units", "count");
        bottom_var->add_att("units", "count");
        caustic_var->add_att("units", "count");
        
        // write eigenrays  
        
        freq_var->put(eigenrays.begin()->frequencies->data().begin(), num_frequencies);

        int record = 0; // current record number
        BOOST_FOREACH(eigenray ray, eigenrays)
        {
            // set record number for each eigenray data element

            eigenray_num_var->set_cur(record);
            intensity_var->set_cur(record);
            phase_var->set_cur(record);
            time_var->set_cur(record);
            source_de_var->set_cur(record);
            source_az_var->set_cur(record);
            target_de_var->set_cur(record);
            target_az_var->set_cur(record);
            surface_var->set_cur(record);
            bottom_var->set_cur(record);
            caustic_var->set_cur(record);
            eigenray_num_var->put(&record, 1);
            ++record;
  
            intensity_var->put(ray.intensity.data().begin(), 1, num_frequencies);
            phase_var->put(ray.phase.data().begin(), 1, num_frequencies);
            time_var->put(&(ray.time), 1);
            source_de_var->put(&(ray.source_de), 1);
            source_az_var->put(&(ray.source_az), 1);
            target_de_var->put(&(ray.target_de), 1);
            target_az_var->put(&(ray.target_az), 1);
            surface_var->put(&(ray.surface), 1);
            bottom_var->put(&(ray.bottom), 1);
            caustic_var->put(&(ray.caustic), 1); 
            
        } // loop over all eigenrays
        ++index;
    } // loop over all fathometers

    // close file

    delete nc_file; // destructor frees all netCDF temp variables
}

/**
 * Gets the envelopes for the sensor receiver_id requested.
 */
envelope_collection::envelope_package sensor_pair_manager::get_envelopes(sensor_query_map sensors)
{
    sensor_pair* pair;
    read_lock_guard guard(_manager_mutex);

    std::set<std::string> keys = find_pairs(sensors);
    envelope_collection::envelope_package envelopes;
    envelopes.reserve(keys.size());
    BOOST_FOREACH(std::string s, keys)
    {
        pair = _map.find(s);
        sensor_pair* pair_data = pair;
        if ( pair_data != NULL ) {
            envelope_collection::reference collection = pair_data->envelopes();
            if ( collection.get() != NULL ) {
                envelopes.push_back(collection.get());
            }
        }
    }
    return envelopes;
}

/**
 * Builds new sensor_pair objects in reaction to notification
 * that a sensor is being added.
 */
void sensor_pair_manager::add_sensor(sensor_model* sensor) {
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
    
    // Add pair as required

    switch ( sensor->mode() )
    {
        case usml::sensors::SOURCE:
            add_multistatic_source(sensor);
            break;
        case usml::sensors::RECEIVER:
            add_multistatic_receiver(sensor);
            break;
        case usml::sensors::BOTH:
            // Check frequency band overlap
            add_monostatic_pair(sensor);

            // add multistatic pairs when multistatic is true 
            if ( sensor->source()->multistatic() ) {
                add_multistatic_source(sensor);    
            }              
            if ( sensor->receiver()->multistatic() ) {
                add_multistatic_receiver(sensor);
            }
            break;
        default:
            break;
    }

    #ifdef USML_DEBUG
        // Print out all pairs
        cout << "sensor_pair_manager:  current pairs" << endl;
        sensor_map_template<std::string, sensor_pair*>::iterator iter;
        for ( iter = _map.begin(); iter != _map.end(); ++iter )
        {
            std::string key  = iter->first;
            cout << "     pair  src_rcv " << key << endl;      
         } 
    #endif
}

/**
 * Removes existing sensor_pair object in reaction to notification
 * that the sensor is about to be deleted.
 */
bool sensor_pair_manager::remove_sensor(sensor_model* sensor) {
    size_t result = 0;
	write_lock_guard guard(_manager_mutex);
	#ifdef USML_DEBUG
		cout << "sensor_pair_manager: remove sensor("
		<< sensor->sensorID() << ")" << endl;
	#endif

	// remove sensorID from the lists of active sources and receivers

	switch (sensor->mode()) {
	case usml::sensors::SOURCE:
        result = _src_list.erase(sensor->sensorID());
		break;
	case usml::sensors::RECEIVER:
        result = _rcv_list.erase(sensor->sensorID());
		break;
	case usml::sensors::BOTH:
        result =_src_list.erase(sensor->sensorID());
        result = _rcv_list.erase(sensor->sensorID());
		break;
	default:
		break;
	}
    // Exit if the sensorID/mode was not found
    if ((int) result == 0 ) return false;

	// Remove pair as required

    switch ( sensor->mode() )
    {
        case usml::sensors::SOURCE:
            remove_multistatic_source(sensor);
            break;
        case usml::sensors::RECEIVER:
            remove_multistatic_receiver(sensor);
            break;
        case usml::sensors::BOTH:
            // Check frequency band overlap
            remove_monostatic_pair(sensor);

            // add multistatic pairs when multistatic is true 
            if ( sensor->source()->multistatic() )
            {
                remove_multistatic_source(sensor);
            }
            if ( sensor->receiver()->multistatic() )
            {
                remove_multistatic_receiver(sensor);
            }
            break;
        default:
            break;
    }
    return true;
}

/**
 * Utility to build a monostatic pair
 */
void sensor_pair_manager::add_monostatic_pair(sensor_model* sensor) {
	sensor_model::id_type sourceID = sensor->sensorID();
    std::string hash_key = generate_hash_key(sourceID, sourceID);
    sensor_pair* pair = new sensor_pair(sensor, sensor); 
	_map.insert(hash_key, pair);
	sensor->add_sensor_listener(pair);
	#ifdef USML_DEBUG
		cout << "   add_monostatic_pair: sensor_pair("
		<< sourceID << "," << sourceID << ")" << endl;
	#endif
}

/**
 * Utility to build a multistatic pair from the source.
 */
void sensor_pair_manager::add_multistatic_source(sensor_model* source) {
	sensor_model::id_type sourceID = source->sensorID();
	BOOST_FOREACH( sensor_model::id_type receiverID, _rcv_list ) {
		if ( sourceID != receiverID ) {
            sensor_model* receiver_sensor = sensor_manager::instance()->find(receiverID);
            if ( receiver_sensor != NULL &&
                    receiver_sensor->receiver()->multistatic() &&
                    frequencies_overlap(source->source()->frequencies(), 
                    receiver_sensor->receiver()->min_active_freq(),
                    receiver_sensor->receiver()->max_active_freq()) )
			{
                std::string hash_key = generate_hash_key(sourceID, receiverID);
                sensor_pair* pair = new sensor_pair(source, receiver_sensor);
                _map.insert(hash_key, pair);
				source->add_sensor_listener(pair);
                receiver_sensor->add_sensor_listener(pair);
				#ifdef USML_DEBUG
					cout << "   add_multistatic_source: sensor_pair("
					<< sourceID << "," << receiverID << ")" << endl;
				#endif
			}
		}
	}
}

/**
 * Utility to build a multistatic pair from the receiver.
 */
void sensor_pair_manager::add_multistatic_receiver(sensor_model* receiver) {
	sensor_model::id_type receiverID = receiver->sensorID();
	BOOST_FOREACH( sensor_model::id_type sourceID, _src_list ) {
		if ( sourceID != receiverID ) { // exclude monostatic case
			sensor_model* source_sensor = sensor_manager::instance()->find(sourceID);
            if ( source_sensor != NULL && 
                    source_sensor->source()->multistatic() &&
                    frequencies_overlap(source_sensor->source()->frequencies(), 
                                        receiver->receiver()->min_active_freq(),
                                        receiver->receiver()->max_active_freq()) )
			{
                std::string hash_key = generate_hash_key(sourceID, receiverID);
                sensor_pair* pair = new sensor_pair(source_sensor, receiver);
                _map.insert(hash_key, pair);
                source_sensor->add_sensor_listener(pair);
				receiver->add_sensor_listener(pair);
				#ifdef USML_DEBUG
					cout << "   add_multistatic_receiver: sensor_pair("
					<< sourceID << "," << receiverID << ")" << endl;
				#endif
			}
		}
	}
}

/**
 * Utility to remove a monostatic pair
 */
void sensor_pair_manager::remove_monostatic_pair(sensor_model* sensor) {
	sensor_model::id_type sourceID = sensor->sensorID();
    std::string hash_key = generate_hash_key(sourceID, sourceID);
	sensor_pair* pair = _map.find(hash_key);
	if (pair != NULL) {
		sensor->remove_sensor_listener(pair);
        delete pair;
		_map.erase(hash_key);
		#ifdef USML_DEBUG
			cout << "   remove_monostatic_pair: sensor_pair("
				 << sourceID << "," << sourceID << ")" << endl;
		#endif
	}
}

/**
 * Utility to remove a multistatic pair from the source.
 */
void sensor_pair_manager::remove_multistatic_source(sensor_model* source) {
	sensor_model::id_type sourceID = source->sensorID();
	BOOST_FOREACH( sensor_model::id_type receiverID, _rcv_list ) {
		if ( sourceID != receiverID ) {
            sensor_model* receiver_sensor = sensor_manager::instance()->find(receiverID);
            if ( receiver_sensor != NULL &&
                    receiver_sensor->receiver()->multistatic() &&
                    frequencies_overlap(source->source()->frequencies(), 
                                        receiver_sensor->receiver()->min_active_freq(),
                                        receiver_sensor->receiver()->max_active_freq()) )
			{
                std::string hash_key = generate_hash_key(sourceID, receiverID);
				sensor_pair* pair = _map.find(hash_key);
				if ( pair != NULL ) {
					source->remove_sensor_listener(pair);
                    receiver_sensor->remove_sensor_listener(pair);
                    delete pair;
					_map.erase( hash_key );
					#ifdef USML_DEBUG
						cout << "   remove_multistatic_source: sensor_pair("
							 << sourceID << "," << receiverID << ")" << endl;
					#endif
				}
			}
		}
	}
}

/**
 * Utility to remove a multistatic pair from the receiver.
 */
void sensor_pair_manager::remove_multistatic_receiver(sensor_model* receiver) {
	sensor_model::id_type receiverID = receiver->sensorID();
	BOOST_FOREACH( sensor_model::id_type sourceID, _src_list ) {
		if ( sourceID != receiverID ) { // exclude monostatic case
			sensor_model* source_sensor = sensor_manager::instance()->find(sourceID);
            if ( source_sensor != NULL && 
                source_sensor->source()->multistatic() && 
                frequencies_overlap(source_sensor->source()->frequencies(), 
                                    receiver->receiver()->min_active_freq(),
                                    receiver->receiver()->max_active_freq()) )
			{
                std::string hash_key = generate_hash_key(sourceID, receiverID);
				sensor_pair* pair = _map.find(hash_key);
				if ( pair != NULL ) {
                    source_sensor->remove_sensor_listener(pair);
					receiver->remove_sensor_listener(pair);
                    delete pair;
					_map.erase( hash_key );
					#ifdef USML_DEBUG
						cout << "   remove_multistatic_receiver: sensor_pair("
							 << sourceID << "," << receiverID << ")" << endl;
					#endif
				}
			}
		}
	}
}

/**
* Utility to determine if two frequency ranges overlap
* Used to determine of a sensor_pair needs to be created.
*/
bool sensor_pair_manager::frequencies_overlap(const seq_vector* src_freq, 
                                                double  rcv_min, double rcv_max) {
    // Get source min and max
    double src_min = *( src_freq->data().begin() );
    double src_max = *( src_freq->data().end() - 1 );

    //// Get receiver min and max
    //double rcv_min = *( rcv_freq->data().begin() );
    //double rcv_max = *( rcv_freq->data().end() - 1 );

    bool overlap = src_min <= rcv_max &&  rcv_min <= src_max;
    
    #ifdef NOOP
        cout << "   frequencies_overlap: src_min( "
            << src_min << " ) - src_max( " << src_max  << ")" << endl;
        cout << "                        rcv_min( "
            << rcv_min << " ) - rcv_max( " << rcv_max << ")" << endl;
        cout << "               overlap: " << ( ( overlap ) ? "yes":"no" ) << endl;
    #endif

    return overlap;
}
