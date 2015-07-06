/**
 * @file wavefront_generator.h
 * Generates eigenrays and eigenverbs for the reverberation model.
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
#include <usml/waveq3d/eigenray_notifier.h>
#include <usml/eigenverb/eigenverb_collection.h>
#include <usml/eigenverb/wavefront_listener.h>

namespace usml {
namespace eigenverb {

using namespace usml::ocean ;
using namespace usml::threads ;
using namespace usml::waveq3d ;
using namespace usml::types ;

/// @ingroup eigenverb
/// @{

/**
 * Generates eigenrays and eigenverbs for the reverberation model.
 * This class inherits the thread_task interface and is used to assemble
 * the all data required by the wave_queue and then run the wave_queue.
 * Once instantiated the class run method is called by passing its pointer
 * into the thread_controller's run method.
 *
 * Several of the static attributes that can be set prior to constructing
 * this class are as follows with there default values:
 * <pre>
 *  wavefront_generator::number_de = 181;              // Number of depression/elevation indices
 *  wavefront_generator::number_az = 18;               // Number of azimuthal indices
 *  wavefront_generator::time_maximum = 90.0;          // Max run time seconds
 *  wavefront_generator::time_step = 0.01;             // Step time i seconds
 *  wavefront_generator::intensity_threshold = -300.0; // dB  Eigenray with intensity values below this are discarded.
 *  wavefront_generator::max_bottom = 999;             // Max number of bottom bounces.
 *  wavefront_generator::max_surface = 999;            // Max number of surface bounces.
 * </pre>
 */

class USML_DECLSPEC wavefront_generator : public thread_task
{
    public:

    /**
     * Constructor
     *
     *  @param ocean shared pointer to the ocean_model required by the wave_queue
     *  @param source_position wposition1 type that contains the lat, lon, alt
     *         of the source sensor.
     *  @param target_positions wposition type that contains the lat, lon, alt
     *         of each of the targets.
     *  @param frequencies pointer to a seq_vector type that contains all the
     *         freq for wave_queue.
     *  @param listener pointer to a sensor_listener class that will receive the
     *         eigenrays and eigenverbs.
     *  @param vertical_beamwidth in decimal degrees of the sensors beam above and
     *         below depression_elevation_angle.
     *         Optional, not used when not provided.
     *  @param depression_elevation_angle decimal degrees offset from the
     *         horizontal plane.
     *         Optional, Defaults to 0 (zero).
     *  @param run_id ID for this propagation run of wave_queue.
     *         Optional. Defaults to 0 (zero).
     */
    wavefront_generator(shared_ptr<ocean_model> ocean, wposition1 source_position,
        const wposition* target_positions, const seq_vector* frequencies, wavefront_listener* listener,
        double vertical_beamwidth = 0.0, double depression_elevation_angle = 0.0, int run_id = 0);

    /**
     * Virtual destructor
     */
    virtual ~wavefront_generator() {
        if (_target_positions != NULL) {
            delete _target_positions;
        }
    }

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
     * Default Number of extra DE rays for the WaveQ3D wavefront
     */
    static int extra_rays;

     /**
     * Default Maximum time (sec) to propagate WaveQ3D wavefront.
     */
    static double time_maximum;

    /**
     * Default Maximum time (sec) to propagate WaveQ3D wavefront.
     */
    static double time_step;

    /**
     * The value of the intensity threshold in dB.
     * Any eigenray or eigenverb with an intensity value that are weaker
     * than this threshold is not sent the listeners.
     * Defaults to -300 dB
     */
    static double intensity_threshold;

    /**
     * The maximum number of bottom bounces.
     * Any eigenray or eigenverb with more than this number
     * of bottom bounces is not sent the listeners.
     * Defaults to 999.
     */
    static int max_bottom ;

    /**
     * The maximum number of surface bounces.
     * Any eigenray or eigenverb with more than this number
     * of surface bounces is not sent the listeners.
     * Defaults to 999.
     */
    static int max_surface ;

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

    /** Source position */
    const wposition1 _source_position;

    /** Targets container */
    // Takes ownership
    const wposition* _target_positions;

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
