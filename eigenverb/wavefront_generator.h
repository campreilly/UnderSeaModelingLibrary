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
#include <usml/waveq3d/eigenray_collection.h>
#include <usml/waveq3d/wave_queue.h>
#include <usml/waveq3d/eigenray_listener.h>
#include <usml/eigenverb/wavefront_listener.h>

namespace usml {
namespace eigenverb {

using namespace usml::ocean ;
using namespace usml::threads ;
using namespace usml::waveq3d ;
using namespace usml::types ;

/// @ingroup eigenverb
/// @{

class USML_DECLSPEC wavefront_generator : public thread_task
{
    public:

    /**
     * Constructor
     *  * @param listener pointer to a sensor_listener.
     */
    wavefront_generator(shared_ptr<ocean_model> ocean, wposition1 source_position,
        wposition target_positions, const seq_vector* frequencies, wavefront_listener* listener,
        double vertical_beamwidth = 0.0, double depression_elevation_angle = 0.0, int run_id = 0);

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
     * Default Number of depression/elevation angles to use in WaveQ3D wavefront.
     */
    static int number_de;

    /**
     * Default Number of AZ angles to use in WaveQ3D wavefront
     */
    static int number_az;

     /**
     * Default Maximum time (sec) to propagate WaveQ3D wavefront.
     */
    static double time_maximum;

    /**
     * Default Maximum time (sec) to propagate WaveQ3D wavefront.
     */
    static double time_step;

     /**
     * The value of the intensity threshold in dB
     * Any eigenray intensity values that are weaker than this
     * threshold are stored for later retrieval.
     */
    static double intensity_threshold;

private:

    /**
     * Default Constructor - Prevent Access
     */
    wavefront_generator();

     /** Mutex to lock multiple properties at once. */
    read_write_lock _lock ;

    /** Set to true when WaveQ3D propagation model task complete. */
    bool _done ;

    /**
     * The ID for the WaveQ3D run.
     */
    const int _run_id;

     /**
      * Number of depression/elevation angles to use in WaveQ3D wavefront.
      */
    const int _number_de;

    /**
     * Number of AZ angles to use in WaveQ3D wavefront
     */
    const int _number_az;

    /**
     * Maximum time (sec) to propagate WaveQ3D wavefront.
     */
    const double _time_maximum;

    /**
     * Time each step (sec) of WaveQ3D wavefront increases.
     */
    const double _time_step;

    /**
     * The value of the intensity threshold in dB
     * Any eigenray intensity values that are weaker than this
     * threshold are not sent the eigenray_listner(s);
     */
    const double _intensity_threshold; //In dB

    /** Depression elevation angle in degrees */
    const double  _depression_elevation_angle;

    /** Vertical beam width in degrees*/
    const double  _vertical_beamwidth;

    /** Source position */
    const wposition1 _source_position;

    /** Targets container */
    const wposition _target_positions;

    /** Frequencies container */
    const seq_vector* _frequencies;

    /** Shared Pointer to the OceanModel. */
    shared_ptr<ocean_model> _ocean;

    /** Pointer to the sensor_listener. */
    wavefront_listener* _wavefront_listener;
};

/// @}
}   // end of namespace eigenverb
}   // end of namespace usml
