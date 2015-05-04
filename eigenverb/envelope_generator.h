/**
 * @file envelope_generator.h
 * Computes reverberation envelopes from eigenverbs.
 */
#pragma once

#include <usml/ocean/ocean_shared.h>
#include <usml/threads/thread_task.h>
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
class USML_DECLSPEC envelope_generator: public thread_task, envelope_notifier {
public:

	/**
	 * Default pulse length of the signal (sec)
	 * Defaults to 1.0 sec.
	 */
	static double pulse_length;

	/**
	 * Default duration of the reverberation envelope (sec).
	 * Defaults to 40 sec.
	 */
	static double time_maximum;

	/**
	 * Default sampling period for the reverberation envelope (sec).
	 * Defaults to 0.1 sec.
	 */
	static double time_step;

	/**
	 * Minimum intensity level for valid reverberation contributions (linear units).
	 * Defaults to -300 dB.
	 */
	static double threshold;

	/**
	 * Initialize model parameters and reserve memory.
	 *
	 * @param transmit_freq	Frequencies at which the source and receiver
	 * 						eigenverbs are computed (Hz).
	 * @param num_azimuths	Number of receiver azimuths in result.
	 * @param num_src_beams	Number of source beams in result.
	 * @param num_rcv_beams Number of receiver beams in result.
	 */
	envelope_generator(
		const seq_vector* transmit_freq,
		const seq_vector* receiver_freq,
		size_t num_azimuths,
		size_t num_src_beams,
		size_t num_rcv_beams,
	    eigenverb_collection::reference src_eigenverbs,
	    eigenverb_collection::reference rcv_eigenverbs ) ;

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
	 * Set to true when this task complete.
	 */
	bool done() {
		return _done;
	}

private:

	/**
	 * Computes the broadband scattering strength for a specific interface.
	 *
	 * @param interface		Interface number of ocean component that is doing
	 * 						the scattering. See the eigenverb class header
	 * 						for documentation on interpreting this number.
	 * @param location      Location at which to compute attenuation.
	 * @param frequencies   Frequencies over which to compute loss. (Hz)
	 * @param de_incident   Depression incident angle (radians).
	 * @param de_scattered  Depression scattered angle (radians).
	 * @param az_incident   Azimuthal incident angle (radians).
	 * @param az_scattered  Azimuthal scattered angle (radians).
	 * @param amplitude     Change in ray strength in dB (output).
	 */
	void scattering( size_t interface, const wposition1& location,
			const seq_vector& frequencies, double de_incident,
			double de_scattered, double az_incident, double az_scattered,
			vector<double>* amplitude);

	/** Set to true when this task complete. */
	bool _done;

	/** Ocean data to use for the envelope calculation. */
	shared_ptr<ocean_model> _ocean;

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
}	// end of namespace eigenverb
}   // end of namespace usml
