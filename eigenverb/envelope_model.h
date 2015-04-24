/**
 * @file envelope_spectrum.h
 * Reverberation envelope spectrum for a single combination of
 * receiver azimuth, source beam number, receiver beam number.
 */
#pragma once

#include <usml/types/seq_linear.h>
#include <usml/threads/smart_ptr.h>
#include <usml/eigenverb/eigenverb.h>
#include <usml/eigenverb/envelope_model.h>

namespace usml {
namespace eigenverb {

using namespace usml::types;
using namespace usml::threads;

/**
 * Reverberation envelope spectrum for a single combination of
 * receiver azimuth, source beam number, receiver beam number.
 * Computes the spectrum as a function of source frequency
 * and envelope frequency.  The envelope frequencies are assumed
 * to be a complex basebanded domain (-fs/2,fs/2] where fs is the
 * sampling rate of the receiver.  Forming the envelope contributions
 * in the frequency domain simplifies the application of Doppler shift.
 *
 *
 * @xref S. Reilly, D. Thibaudeau, T. Burns, "Fast computation of
 * reverberation using Gaussian beam reflections," Report to NAWCTSD,
 * October 20, 2014.
 */
class USML_DECLSPEC envelope_model {

public:

	/**
	 * Reserve the memory used to store the results of this calculation.
	 *
	 * @param source_freq	Frequencies at which the source and receiver
	 * 						eigenverbs are computed (Hz).
	 * @param envelope_freq	Frequencies at which the sensor_pair's
	 * 						reverberation envelopes are computed (Hz).
	 * @param threshold		Minimum energy level for valid reverberation
	 * 						contributions (linear units).
	 */
	envelope_model(
		shared_ptr<seq_vector> source_freq,
		shared_ptr<seq_vector> envelope_freq,
		double threshold
	) :
		_source_freq(source_freq),
		_envelope_freq(envelope_freq),
		_threshold( threshold ),
		_pressure(source_freq->size(), envelope_freq->size())
	{
	}

	/**
	 * Default constructor.
	 */
	~envelope_model() {}

	/**
	 * Adds a single combination of source and receiver eigenverbs
	 * to this spectrum.  Assumes that the source and receiver eigenverbs
	 * have already been interpolated onto the sensor_pair's
	 * frequency domain.  Having calling routine compute the scattering
	 * coefficient for this combination of eigenverbs, save this class
	 * from having to know anything about the ocean.
	 *
	 * @param src_verb		Eigenverb contribution from the source.
	 * @param rcv_verb		Eigenverb contribution from the receiver.
	 * @param scatter		Scattering strength coefficient for this
	 * 						combination of eigenverbs (ratio).
	 * @param pulse_length	Duration of the transmit pulse (sec).
	 */
	void add_contribution(
			const eigenverb& src_verb, const eigenverb& rcv_verb,
			const vector<double>& scatter, double pulse_length ) ;

private:

	/**
	 * Compute the total energy and duration of the overlap between
	 * two eigenverbs. Implements the analytic solution for energy of
	 * the bistatic reverberation contribution from eqn. (28) ans (29)
	 * in the paper.  Computes the duration from eqn. (45) and (33).
	 *
	 * @param src_verb		Eigenverb contribution from the source.
	 * @param rcv_verb		Eigenverb contribution from the receiver.
	 * @param scatter		Scattering strength coefficient for this
	 * 						combination of eigenverbs (ratio).
	 * @param pulse_length	Duration of the transmit pulse (sec).
	 * @param energy		Total energy of the overlap (linear units, output).
	 * @param duration		Duration of the overlap (sec, output).
	 * @return				False if energy below threshold.
	 */
	bool compute_overlap(
			const eigenverb& src_verb, const eigenverb& rcv_verb,
			const vector<double>& scatter, double pulse_length,
			vector<double>* energy, vector<double>* duration ) ;

	/**
	 * Adds this contribution to the reverberation envelope spectrum.
	 * Each contribution has a Gaussian shape with a bandwidth
	 * defined by the inverse of the duration. Time delay is applied
	 * as a phase change in the complex spectrum.
	 * Loops over the source and envelope frequencies.
	 *
	 * @param src_verb		Eigenverb contribution from the source.
	 * @param rcv_verb		Eigenverb contribution from the receiver.
	 * @param energy		Total energy of the overlap (linear units).
	 * @param duration		Duration of the overlap (sec).
	 */
	void compute_spectrum(
			const eigenverb& src_verb, const eigenverb& rcv_verb,
			const vector<double>& energy, const vector<double>& duration ) ;

	/**
	 * Frequencies at which the source and receiver eigenverbs are computed (Hz).
	 */
	shared_ptr<seq_vector> _source_freq ;

	/**
	 * Frequencies at which the sensor_pair's reverberation envelopes
	 * are computed (Hz). Assumed to be a complex basebanded
	 * domain (-fs/2,fs/2] where fs is the sampling rate of the receiver.
	 */
	shared_ptr<seq_vector> _envelope_freq ;

	/**
	 * Minimum energy level for valid reverberation contributions
	 * (linear units).
	 */
	double _threshold ;

	/**
	 * Complex pressure at each point in reverberation spectrum.
	 * Each row represents a specific source frequency.
	 * Each column represents a specific envelope frequency.
	 */
	matrix< complex<double> > _pressure;
};

}   // end of namespace eigenverb
}   // end of namespace usml
