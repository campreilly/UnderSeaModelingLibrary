/**
 * @file wavefront_generator.h
 */
#pragma once

#include <iostream>
#include <fstream>

#include <boost/shared_ptr.hpp>


#include <usml/types/seq_data.h>
#include <usml/threads/thread_task.h>
#include <usml/ocean/ocean_shared.h>
#include <usml/waveq3d/proploss.h>
#include <usml/waveq3d/wave_queue.h>
#include <usml/waveq3d/eigenray_listener.h>
#include <usml/eigenverb/wavefront_listener.h>

namespace usml {
namespace eigenverb {

using namespace usml::ocean ;
using namespace usml::threads ;
using namespace usml::waveq3d ;

/// @ingroup eigenverb
/// @{

class USML_DECLSPEC wavefront_generator : public thread_task
{
    public:

    /**
     * Default Constructor
     */
    wavefront_generator();

    /**
     * Virtual destructor
     */
    virtual ~wavefront_generator() {}

    /**
     * Overloads of the thread_task run method.
     * Executes the WaveQ3D propagation model.
     */
    virtual void run();

    /**
     * Set to true when WaveQ3D propagation model task complete.
     */
    bool done() {
        read_lock_guard guard(_lock);
        return _done ;
    }

    /**
     * Sets runID for the WaveQ3D run.
     * @param runID
     */
    void runID(int runID) {
        _runID = runID;
    }

    /**
     * Sets the maximum range for the WaveQ3D run.
     * @param range_maximum in meters
     */
    void range_maximum(double range_maximum) {
        _range_maximum = range_maximum;
    }

    /**
     * Sets the intensity_threshold for the WaveQ3D run.
     * @param intensity_threshold in dB
     */
    void intensity_threshold(double intensity_threshold) {
        _intensity_threshold = intensity_threshold;
    }

    /**
     * Sets the _depression_elevation_angle for the WaveQ3D run.
     * @param _depression_elevation_angle in radians
     */
    void depression_elevation_angle(double depression_elevation_angle) {
        _depression_elevation_angle = depression_elevation_angle;
    }

    /**
     * Sets the _vertical_beamwidth for the WaveQ3D run.
     * @param _vertical_beamwidth in radians
     */
    void vertical_beamwidth(double vertical_beamwidth) {
        _vertical_beamwidth = vertical_beamwidth;
    }

    /**
     * Sets the _sensor_position for the WaveQ3D run.
     * @param sensor_position wposition1 type
     */
    void sensor_position(wposition1 sensor_position) {
        _sensor_position = sensor_position;
    }

    /**
     * Sets the _targets for the WaveQ3D run.
     * @param targets wposition container type.
     */
    void targets(wposition targets) {
        _targets = targets;
    }

    /**
     * Sets the frequencies for the WaveQ3D run.
     * @param frequencies pointer to a seq_data type
     */
    void frequencies(const seq_vector* frequencies) {
        _frequencies = frequencies;
    }

    /**
     * Sets the ocean for the WaveQ3D run.
     * @param ocean shared_pointer to an ocean_model
     */
    void ocean(shared_ptr<ocean_model> ocean) {
        _ocean = ocean;
    }

    /**
     * Sets the sensor_listener object
     * @param listener pointer to a sensor_listener.
     */
    void wavefront_listener(wavefront_listener* listener) {
        _wavefront_listener = listener;
    }

    /**
     * Pulse length of the signal (sec)
     */
    static double pulse_length;

    /**
     * Maximum time (sec) to propagate WaveQ3D wavefront.
     */
    static double time_maximum;

    /**
     * Maximum time (sec) to propagate WaveQ3D wavefront.
     */
    static double time_step;

    /**
     * Number of depression/elevation angles to use in WaveQ3D wavefront.
     */
    static int number_de;

    /**
     * Number of AZ angles to use in WaveQ3D wavefront
     */
    static int number_az;

private:


    /** Set to true when WaveQ3D propagation model task complete. */
    bool _done ;

    /** Mutex to lock multiple properties at once. */
    read_write_lock _lock ;

    /**
     * The ID for the WaveQ3D run.
     */
    int     _runID;

    /**
     * Maximum range (meters) to propagate WaveQ3D wavefront.
     */
    double  _range_maximum;

    /**
     * The value of the intensity threshold in dB
     * Any eigenray intensity values that are weaker than this
     * threshold are not sent the eigenray_listner(s);
     */
    double  _intensity_threshold; //In dB

    /** Depression elevation angle in degrees */
    double  _depression_elevation_angle;

    /** Vertical beam width in degrees*/
    double  _vertical_beamwidth;

    /** Sensor position */
    wposition1 _sensor_position;

    /** Targets container */
    wposition _targets;

    /** Frequencies container */
    const seq_vector* _frequencies;

    /** Shared Pointer to the OceanModel. */
    shared_ptr<ocean_model> _ocean;

    /** Pointer to the sensor_listener. */
    usml::eigenverb::wavefront_listener* _wavefront_listener;
};

/// @}
}   // end of namespace eigenverb
}   // end of namespace usml
