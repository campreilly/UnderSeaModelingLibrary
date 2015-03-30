/**
 *  @file sensor.cc
 *  Implementation of the Class sensor
 *  Created on: 10-Feb-2015 12:49:09 PM
 */

#include "sensor.h"

using namespace usml::sensors;

/**
 * Constructor Uses the paramID and mode, looks up source and/or receiver from
 * there associated map.
 */
sensor::sensor(const id_type sensorID, const sensor_params::id_type paramsID, const xmitRcvModeType mode,
	const wposition1 position, const double pitch, const double yaw, const double roll, const std::string description)
	:	_sensorID(sensorID),
		_paramsID(paramsID),
		_src_rcv_mode(mode),
		_position(position),
		_pitch(pitch),
		_yaw(yaw),
		_roll(roll),
		_fathometers(NULL),
		_eigenverbs(NULL),
        _description(description)
{
    _source.reset();
    _receiver.reset();
    source_params_map* source_map = source_params_map::instance();
    receiver_params_map* receiver_map = receiver_params_map::instance();

    switch (mode)
    {
        default:
            assert(false);
            break;
        case usml::sensors::SOURCE:
        {
            source_params::reference sp = source_map->find(_paramsID);
            if ( sp.get() == NULL) {
                throw "source_params not found";
            } else {
                _source = sp;
            }
            break;
        }
        case usml::sensors::RECEIVER:
        {
            receiver_params::reference rp = receiver_map->find(_paramsID);
            if (rp.get() == NULL) {
                throw "receiver_params not found";
            } else {
                _receiver = rp;
            }
            break;
        }
        case usml::sensors::BOTH:
        {
            source_params::reference sp = source_map->find(_paramsID);
            if (sp.get() == NULL) {
                throw "source_params not found";
            } else {
                _source = sp;
            }
            receiver_params::reference rp = receiver_map->find(_paramsID);
            if (rp.get() == NULL) {
                throw "receiver_params not found";
            } else {
                _receiver = rp;
            }
        }
    }
}

/**
 * Destructor
 */
sensor::~sensor()
{
    // Kill operating task
    _wavefront_task->abort();
}

/**
 * Sets the latitude of the sensor. Expects latitude to be in decimal degrees.
 * @param latitude    latitude
 */
void sensor::latitude(double latitude)
{
    _position.latitude(latitude);
}

/**
 * Get method for the latitude of the sensor.
 * @return latitude in decimal degrees.
 */
double sensor::latitude()
{
    return _position.latitude();
}

/**
 * Sets the longitude of the sensor. Expects longitude to be in decimal degrees.
 * @param longitude    longitude
 */
void sensor::longitude(double longitude)
{
    _position.longitude(longitude);
}

/**
 * Get method for the longitude of the sensor.
 * @return longitude in decimal degrees.
 */
double sensor::longitude()
{
    return _position.longitude();
}

/**
 * Sets the depth of the sensor. Expects depth to be in meters.
 * @param depth    depth
 */
void sensor::depth(double depth)
{
    // Set wposition1.altitude
    _position.altitude(-depth);
}

/**
 * Get method for the depth of the sensor.
 * @return depth of the sensor in meters.
 */
double sensor::depth()
{
    return -(_position.altitude());
}

/**
 * Checks to see if new position, pitch and yaw have changed enough
 * to require a new WaveQ3D run.
 */
bool sensor::check_thresholds(wposition1 position, double pitch, double yaw, double roll)
{
    return false;

}

/**
 * Initialize the wave_generator thread to start the waveq3d model.
 */
void sensor::init_wave_generator()
{


    // Create the wavefront_generator
    wavefront_generator* generator = new wavefront_generator();

    // Populate waveq3d settings
    generator->wavefront_listener(this);
    generator->sensor_position(_position);

    // Get the targets
    wposition targets = sensor_targets();
    generator->targets(targets);

    // Get the frequencies
    generator->frequencies(_source->frequencies());
    generator->ocean(ocean_shared::current());

    // Make wavefront_generator a wavefront_task, with use of shared_ptr
    _wavefront_task = thread_task::reference(generator);

    // Pass in to thread_pool
    thread_controller::instance()->run(_wavefront_task);
}

/**
 * Updates the sensor data, checks position, pitch, yaw, and roll thresholds to determine
 * if new wave_generator needs to be run, then kicks off the waveq3d model.
 * @param force_run defaults to false, set true to force new run
 * 
 * @param position  updated position data
 * @param pitch     updated pitch value
 * @param yaw       updated yaw value
 * @param roll      updated roll value
 * @param force_update
 */
void sensor::update_sensor(wposition1 position, double pitch, double yaw, double roll, bool force_run)
{
	if ( !force_run )
	{
	     if (!check_thresholds(position, pitch, yaw, roll)) {
            return;
        }
	}
	_position = position;
	_pitch = pitch;
	_yaw = yaw;
	_roll = roll;
	init_wave_generator();

	return;
}

/**
 * Add a sensor_listener to the _sensor_listeners list
 * @param listener
 */
bool sensor::add_sensor_listener(sensor_listener* listener)
{  
    std::list<sensor_listener*>::iterator iter = find(_sensor_listeners.begin(), _sensor_listeners.end(), listener);
    if ( iter != _sensor_listeners.end() )
    {
        return false;
    }   
    _sensor_listeners.push_back(listener);
    return true;  
}

/**
 * Remove a sensor_listener from the _sensor_listeners list
 * @param listener
 */
bool sensor::remove_sensor_listener(sensor_listener* listener)
{
    std::list<sensor_listener*>::iterator iter = find(_sensor_listeners.begin(), _sensor_listeners.end(), listener);
    if ( iter == _sensor_listeners.end() )
    {
        return false;
    }
    else
    {
        _sensor_listeners.erase(remove(_sensor_listeners.begin(), _sensor_listeners.end(), listener));
    }
    return true;
}

/**
 * For each sensor_listener in the _sensor_listeners list call the
 * update_eigenverbs method of each registered class.
 */
bool sensor::update_eigenverb_listeners()
{

    for ( std::list<sensor_listener*>::iterator iter = _sensor_listeners.begin();
        iter != _sensor_listeners.end(); ++iter )
    {
        sensor_listener* sensor_pair_ = *iter;
        sensor_pair_->update_eigenverbs(this);
    }

    return ( _sensor_listeners.size() > 0 );
}

/**
 * For each sensor_listener in the _sensor_listeners list call the
 * update_fathometers method of each registered class.
 */
bool sensor::update_fathometer_listeners()
{

    for ( std::list<sensor_listener*>::iterator iter = _sensor_listeners.begin();
        iter != _sensor_listeners.end(); ++iter )
    {
        sensor_listener* listener = *iter;
        listener->update_fathometers(this);
    }

    return ( _sensor_listeners.size() > 0 );
}

/**
 * For each sensor_listener in the _sensor_listeners list call the
 * sensor_complement method to get a list of the complements.
 */
std::list<sensor*> sensor::sensor_complements()
{
    std::list<sensor*> complements;

    for ( std::list<sensor_listener*>::iterator iter = _sensor_listeners.begin();
        iter != _sensor_listeners.end(); ++iter )
    {
        sensor_listener* listener = *iter;
        sensor* sensor_ = listener->sensor_complement(this);
        complements.push_back(sensor_);
    }

    return complements;
}

wposition sensor::sensor_targets()
{
    // Using the sensor_complements call we get all the targets
    // Here we reach out to all the sensor_pairs via there listeners
    std::list<sensor*> complements = sensor_complements();

    std::list<sensor*>::iterator iter;
    sensor* target;
    wposition1 target_position;
    wposition targets(complements.size(), 1 , 0.0, 0.0, 0.0);

    int row = -1;
    for (iter = complements.begin(); iter != complements.end(); ++iter) {
        ++row;
        target = *iter;
        target_position = target->position();
        targets.latitude(row, 0, target_position.latitude());
        targets.longitude(row, 0, target_position.longitude());
        targets.altitude(row, 0, target_position.altitude());
    }

    return targets;
}
