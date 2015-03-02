///////////////////////////////////////////////////////////
//  sensor_pair_map.h
//  Implementation of the Class sensor_pair_map
//  Created on:      26-Feb-2015 5:46:35 PM
//  Original author: Ted Burns, AEgis Technologies
///////////////////////////////////////////////////////////

#pragma once

#include <usml/usml_config.h>
#include <usml/types/wposition1.h>
#include <usml/waveq3d/proploss.h>
#include <usml/sensors/sensor.h>
#include <usml/sensors/sensorIDType.h>
#include <usml/sensors/sensor_listener.h>
#include <usml/eigenverb/data_collections.h>
#include <usml/eigenverb/eigenverb_collection.h>
#include <usml/eigenverb/envelope_collection.h>

namespace usml {
namespace sensors {

using namespace waveq3d ;
using namespace eigenverb ;

/// @ingroup sensors
/// @{

/**
 * Provides storage for all the sensor pair's in use by the USML. 
 * The sensor pair contains a source, receiver acoustic pair.
 * The sensor_pair_map stores pointers to the data required for each sensor pair.
 * This class inherits it's base map from the std::map.
 * The sensor_pair_map contists of two keys and a pointer to the data_collections. 
 * The first one represents the source an the second key represents the reciever. 
 */
class USML_DECLSPEC sensor_pair_map : public sensor_listener
{

public:

    /**
    * Singleton Constructor - Creates sensor_pair_map instance just once, then
    * Accessible everywhere.
    * @return  pointer to the instance of the sensor_pair_map.
    */
    static sensor_pair_map* instance();
	
    /**
    * Destructor - Deletes all pointers its has taken ownership to.
    */
    virtual ~sensor_pair_map();


    /**
    *
    * @return envelopes
    */
    envelope_collection* get_envelopes();
    /**
    *
    * @return proploss
    */
    proploss* get_fathometers();

  
    /**
    *
    * @param fathometers
    */
    void update_fathometers(proploss* fathometers);

    /**
    *
    * @param envelopes
    */
    void update_envelopes(envelope_collection* envelopes);

    /**
    *
    * @param eigenverbs
    */
    void update_eigenverbs(eigenverb_collection* eigenverbs);


    /**
    * Overloaded sensor_changed via the sensor_listener interface
    * @param sensorID
    */
    bool sensor_changed(sensorIDType sensorID);

	/**
	 * 
	 * @param sensor
	 */
	void add_sensor(sensor sensor);
	/**
	 * 
	 * @param sensor
	 */
    void remove_sensor(sensor sensor);

protected:
    /**
    * Default Constructor
    *   Protected to prevent access.
    */
    sensor_pair_map() {}

 private: 
    /**
    * Prevent access to copy constructor
    */
     sensor_pair_map(sensor_pair_map const&);

    /**
    * Prevent access to assignment operator
    */
     sensor_pair_map& operator=( sensor_pair_map const& );

    /**
    * The singleton access pointer.
    */
     static sensor_pair_map* _instance;

    /**
    * The std::map that stores that uses the first sensorID as a key and
    * the payload of the map is another std::map that uses the second sensorID as
    * the second key and the sensorData as the second payload.  
    */
    std::map < std::pair<sensorIDType, sensorIDType>, data_collections& > _map ;

};

/// @}
}  // end of namespace sensors
}  // end of namespace usml