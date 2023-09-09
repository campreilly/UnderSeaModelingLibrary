/**
 * @file wavefront_generator.h
 * Generates eigenrays and eigenverbs for the reverberation model.
 */
#pragma once

#include <usml/ocean/ocean_model.h>
#include <usml/threads/thread_task.h>
#include <usml/types/seq_vector.h>
#include <usml/types/wposition.h>
#include <usml/types/wposition1.h>
#include <usml/usml_config.h>

#include <boost/numeric/ublas/matrix.hpp>

namespace usml {
namespace sensors {
class sensor_model;
}
}  // namespace usml

namespace usml {
namespace wavegen {

using namespace usml::ocean;
using namespace usml::sensors;
using namespace usml::threads;
using namespace usml::types;

/// @ingroup wavegen
/// @{

/**
 * Background task to recompute eigenrays and eigenverbs when sensor motion
 * exceeds position or orientation thresholds. If an existing
 * wavefront_generator is running for this sensor, that task is aborted before
 * the new background task is created. Results are stored in the sensor_model
 * that invoked this background task, unless the task is aborted prior to
 * completion.
 */
class USML_DECLSPEC wavefront_generator : public thread_task {
   public:
    /**
     * Construct wavefront generator for a specific sensor.
     *
     * @param source        	Reference to the source for this wavefront.
     * @param target_positions 	Positions of targets for this wavefront.
     * @param targetIDs  		IDs of targets for this wavefront.
     * @param frequencies   	List of frequencies to use in wavefront calc.
     * @param de_fan        	List of depression/elevation angles to use.
     * @param az_fan        	List of azimuthal angles to use in wavefront.
     * @param time_step     	Time step between wavefronts (sec).
     * @param time_maximum  	Maximum time to propagate wavefront (sec).
     * @param intensity_threshold Intensity threshold in wavefront (dB).
     * @param max_bottom    	The maximum number of bottom bounces.
     * @param max_surface   	The maximum number of surface bounces.
     */
    wavefront_generator(sensor_model* source, const wposition& target_positions,
                        const matrix<int>& targetIDs,
                        seq_vector::csptr frequencies, seq_vector::csptr de_fan,
                        seq_vector::csptr az_fan, double time_step,
                        double time_maximum, double intensity_threshold,
                        int max_bottom, int max_surface);

    /**
     * Executes the WaveQ3D propagation model to generate eigenrays and
     * eigenverbs. Updates the eigenrays and eigenverbs of this source using
     * the source's update_wavefront_data() function when complete.
     */
    virtual void run();

   private:
    /// Reference to the shared ocean at the time of invocation.
    /// Cached to avoid change while the calculation is being performed.
    ocean_model::csptr _ocean;

    /// Reference to the source of this wavefront.
    /// Also the destination of wavefront_listener callbacks.
    sensor_model* _source;

    /// Position of the source at the time of invocation.
    /// Cached to avoid change while the calculation is being performed.
    const wposition1 _source_position;

    /// Position of the targets at the time of invocation.
    /// Cached to avoid change while the calculation is being performed.
    wposition _target_positions;

    /// List of platform ID numbers for each entry in target_positions.
    /// Cached to avoid change while the calculation is being performed.
    matrix<int> _targetIDs;

    /// List of frequencies to use in wavefront calculation.
    seq_vector::csptr _frequencies;

    /// List of depression/elevation angles to use in wavefront calculation.
    seq_vector::csptr _de_fan;

    /// List of azimuthal angles  to use in wavefront calculation.
    seq_vector::csptr _az_fan;

    /// Time step between wavefronts (sec).
    const double _time_step;

    /// Maximum time to propagate wavefront (sec).
    const double _time_maximum;

    /**
     * The value of the wavefront intensity threshold in dB.
     * Any eigenray or eigenverb with an intensity value that are weaker
     * than this threshold is not sent the listeners.
     * Defaults to -300 dB
     */
    const double _intensity_threshold;

    /**
     * The maximum number of bottom bounces.
     * Any eigenray or eigenverb with more than this number
     * of bottom bounces is not sent the listeners.
     * Defaults to 999.
     */
    const int _max_bottom;

    /**
     * The maximum number of surface bounces.
     * Any eigenray or eigenverb with more than this number
     * of surface bounces is not sent the listeners.
     * Defaults to 999.
     */
    const int _max_surface;
};

/// @}
}  // namespace wavegen
}  // namespace usml
