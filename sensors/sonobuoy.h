/**
 *  @file sonobuoy.h
 *  Definition of the Class sonobuoy
 *  Created on: 10-Feb-2015 12:49:09 PM
 */

#pragma once

#include <usml/sensors/sensor_model.h>

namespace usml {
namespace sensors {

/**
 * @author tedburns
 * @version 1.0
 * @created 10-Feb-2015 12:49:09 PM
 */
class USML_DECLSPEC sonobuoy : public sensor_model
{

public:

	/** Construct a new instance of a specific sonobuoy type. */
	sonobuoy( sensor_model::id_type sensorID, sensor_params::id_type paramsID,
			const std::string& description = std::string() ) ;

	/** Virtual destructor. */
	virtual ~sonobuoy() {}

	/**
	 * Acoustic frequency band to use on this buoy.
	 * Counts from zero so that if the buoy has 5 frequency bands,
	 * these bands are labeled 0 through 4.  A value of -1 indicates
	 * that the frequency band is not currently set.
	 */
	int frequency_band() const {
		return _frequency_band;
	}

	/** Acoustic frequency band to use on this buoy.*/
	void frequency_band(int frequency_band) {
		_frequency_band = frequency_band;
	}

	/**
	 * Radio channel used to connect this buoy's aircraft.
	 * Counts from zero so that if the buoy has 100 radio channels,
	 * these channels are labeled 0 through 99.  A value of -1 indicates
	 * that the radio channel is not currently set.
	 */
	int radio_channel() const {
		return _radio_channel;
	}

	/** Radio channel used to connect this buoy's aircraft. */
	void radio_channel(int radio_channel) {
		_radio_channel = radio_channel;
	}

	/**
	 * True when the buoy is ready to communicate with the aircraft.
	 * Checks the status of radio_channel and frequency_band before returning
	 * true.
	 */
	bool uplink_enabled() const {
		return _uplink_enabled && _frequency_band >= 0 && _radio_channel >= 0 ;
	}

	/** True when the buoy is ready to communicate with the aircraft. */
	void uplink_enabled(bool uplink_enabled) {
		_uplink_enabled = uplink_enabled;
	}

private:

	/** Acoustic frequency band to use on this buoy.*/
	int _frequency_band;

	/** Radio channel used to connect this buoy's aircraft. */
	int _radio_channel;

	/** True when the buoy is ready to communicate with the aircraft. */
	bool _uplink_enabled;
};

} // end of namespace sensors
} // end of namespace usml
