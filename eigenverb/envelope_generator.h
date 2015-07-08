/**
 * @file envelope_generator.h
 * Computes reverberation envelopes from eigenverbs.
 */
#pragma once

#include <usml/ocean/ocean_shared.h>
#include <usml/threads/thread_task.h>
#include <usml/sensors/sensor_manager.h>
#include <usml/sensors/sensor_model.h>
#include <usml/sensors/sensor_params.h>
#include <usml/eigenverb/eigenverb_collection.h>
#include <usml/eigenverb/eigenverb_interpolator.h>
#include <usml/eigenverb/envelope_notifier.h>

namespace usml {
namespace eigenverb {

using namespace usml::ocean;
using namespace usml::threads;

/// @ingroup eigenverb
/// @{

/**
 * Computes reverberation envelopes from eigenverbs.
 * Combines eigenverbs to create the reverberation envelope,
 * as a function of travel time, frequency, and receiver beam number,
 * for a fixed set of azimuthal directions around each receiver.
 * An overlap is computed between each receiver eigenverb and
 * all of the source eigenverbs in its vicinity.  This overlap creates
 * a Gaussian reverberation envelope contribution in the time domain.
 *
 * The reverberation envelope contributions for each receiver azimuth
 * are incoherently power summed.  Beam patterns are applied to the
 * eigenverbs during envelope generation.  This allows a single pair of
 * eigenverbs to create separate envelopes for each receiver beam.
 * Eigenverbs and envelopes are computed as functions of frequency
 * so that the pre-computed reverberation results can be applied
 * to a variety of transmitted waveforms in the sonar training system.
 *
 * Invoked as a background thread_task by the sensor_pair for
 * a specific source/receiver combination, whenever one of the sensors
 * updates its eigenverbs. If an existing envelope_generator is running for
 * this sensor_pair, that task is aborted before the new envelope_generator
 * is created.
 */
class USML_DECLSPEC envelope_generator: public thread_task, public envelope_notifier {
public:

    /**
     * Minimum intensity level for valid reverberation contributions (linear units).
     * Defaults to the linear equivalent of -300 dB.
     */
    static double intensity_threshold;

    /**
     * Maximum distance between the peaks of the source and receiver eigenverbs.
     * Specified as a ratio of distances relative to the receiver
     * eigenverb's length and width. Defaults to 6.0.
     */
    static double distance_threshold;

    /**
     * Constructor - Initialize model parameters and reserve memory.
     *
     * @param sensor_pair       Pointer to the sensor_pair that instantiated this class
     * @param initial_time      Start time offset to used calculate the envelope data.
     * @param src_freq_first    Index of the first intersecting frequency of the
     *                          source frequencies seq_vector.  Used to map
     *                          source eigenverbs onto envelope_freq values.
     * @param num_azimuths      Number of receiver azimuths in result.
     */

    envelope_generator(
        sensor_pair* sensor_pair,
        double initial_time,
        size_t src_freq_first,
        size_t num_azimuths ) ;

    /**
     * Virtual destructor
     */
    virtual ~envelope_generator() {
    }

    /**
     * Executes the Eigenverb reverberation model.  For each receiver eigenverb,
     * it loops through the list of source eigenverbs looking for overlaps.
     *
     * First, it computes the great circle range and bearing of the source
     * relative to the receiver.  The combination is skipped if the location
     * of the source (its peak intensity) is more than three (3) times the
     * length/width of the receiver eigenverb,
     * Next, it computes the scattering strength and beam patterns for
     * this source/receiver combination.
     * Finally, it uses the evelope_collection.add_contribution() method
     * to add this this source/receiver combination to the reverberation
     * envelopes.
     */
    virtual void run() ;

    /**
     * Time axis for reverberation calculation.
     */
    static const seq_vector* travel_time() {
        write_lock_guard guard(_property_mutex);
        return _travel_time.get() ;
    }

    /**
     * Resets time axis for reverberation calculation.
     */
    static void travel_time( const seq_vector* time ) {
        write_lock_guard guard(_property_mutex);
        _travel_time.reset( time->clone() ) ;
    }

    /**
     * Set to true when this task complete.
     */
    bool done() const {
        return _done;
    }

private:

    /**
     * Computes the beam_gain matrix
     *
     * @param beam_list Location at which to compute attenuation.
     * @param freq      Frequencies to get beam levels (Hz)
     * @param de_rad    Depression incident angle (radians).
     * @param az_rad    Azimuthal incident angle  (radians).
     * @param orient    orientation
     */
    matrix<double> beam_gain( sensor_params::beam_pattern_list beam_list,
            const seq_vector* freq, double de_rad, double az_rad,
            orientation orient);

    /**
     * Computes the broadband scattering strength for a specific interface.
     * Checks that the scattering strength is greater than the
     * intensity_threshold.  This check allows the model to skip interfaces
     * with trivial scattering strengths.
     *
     * @param interface_num Interface number of ocean component that is doing
     *                      the scattering. See the eigenverb class header
     *                      for documentation on interpreting this number.
     * @param location      Location at which to compute attenuation.
     * @param frequencies   Frequencies over which to compute loss. (Hz)
     * @param de_incident   Depression incident angle (radians).
     * @param de_scattered  Depression scattered angle (radians).
     * @param az_incident   Azimuthal incident angle (radians).
     * @param az_scattered  Azimuthal scattered angle (radians).
     * @param amplitude     Change in ray strength in dB (output).
     * @return              True if any scattering strength is above the threshold.
     */
    bool scattering( size_t interface_num, const wposition1& location,
            const seq_vector& frequencies, double de_incident,
            double de_scattered, double az_incident, double az_scattered,
            vector<double>* amplitude);

    /**
     * The mutex for static properties.
     */
    static read_write_lock _property_mutex;

    /**
     * Time axis for reverberation calculation.  Defaults to
     * a linear sequence out to 400 sec with a sampling period of 0.1 sec.
     */
    static unique_ptr<const seq_vector> _travel_time ;

    /** Set to true when this task complete. */
    bool _done;

    /** Start time offset used to calculate envelope data. */
    double _initial_time;

    /** Ocean data to use for the envelope calculation. */
    shared_ptr<ocean_model> _ocean;

    /**
     * The sensor pair that instantiated this class
     */
    sensor_pair* _sensor_pair;
    
    /**
     * Source Beam Pattern List.
     */
    sensor_params::beam_pattern_list _src_beam_list;

    /**
     * Receiver Beam Pattern List.
     */
    sensor_params::beam_pattern_list _rcv_beam_list;
    

    /**
     * Interface collisions for wavefront emanating from the source.
     */
    eigenverb_collection::reference _src_eigenverbs;

    /**
     * Interface collisions for wavefront emanating from the receiver.
     */
    eigenverb_collection::reference _rcv_eigenverbs;

    /**
     * Utility used to interpolate eigenrays.
     */
    eigenverb_interpolator _eigenverb_interpolator ;

    /**
     * Collection of envelopes generated by this calculation.
     */
    envelope_collection::reference _envelopes ;

} ;

/// @}
}    // end of namespace eigenverb
}   // end of namespace usml
