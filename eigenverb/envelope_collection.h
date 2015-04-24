/**
 * @file envelope_collection.h
 */
#pragma once

#include <usml/eigenverb/envelope_model.h>
#include <usml/types/seq_linear.h>

namespace usml {
namespace eigenverb {

using namespace usml::types;
using namespace usml::threads;

/**
 * Manage the memory for a large collection of reverberation envelope_models.
 * Stores separate values for each receiver azimuth, source beam,
 * and receiver beam in the result of a envelope_generator calculation.
 */
class USML_DECLSPEC envelope_collection {

public:

	// shared pointer
	typedef boost::shared_ptr<envelope_collection> reference;

	/**
	 * Reserve memory in which to store results as a series of
	 * nested dynamic arrays.
	 *
	 * @param source_freq	Frequencies at which the source and receiver
	 * 						eigenverbs are computed (Hz).
	 * @param envelope_freq	Frequencies at which the sensor_pair's
	 * 						reverberation envelopes are computed (Hz).
	 * @param threshold		Minimum energy level for valid reverberation
	 * 						contributions (linear units).
	 * @param num_azimuths	Number of receiver azimuths in result.
	 * @param num_src_beams	Number of source beams in result.
	 * @param num_rcv_beams Number of receiver beams in result.
	 */
	envelope_collection(
		shared_ptr<seq_vector> source_freq,
		shared_ptr<seq_vector> envelope_freq,
		double threshold,
		size_t num_azimuths,
		size_t num_src_beams,
		size_t num_rcv_beams ) ;

	/**
	 * Delete dynamic memory in each of the nested dynamic arrays.
	 */
	~envelope_collection();

	/** Number of receiver azimuths in result. */
	size_t num_azimuths() {
		return _num_azimuths;
	}

	/** Number of receiver azimuths in result. */
	size_t num_src_beams() {
		return _num_src_beams;
	}

	/** Number of receiver azimuths in result. */
	size_t num_rcv_beams() {
		return _num_rcv_beams;
	}

	/**
	 * Intensity time series for one combination of parameters.
	 *
	 * @param azimuth	Receiver azimuth number.
	 * @param src_beam	Source beam number.
	 * @param rcv_beam	Receiver beam number
	 * @param freq		Frequency index number.
	 */
	const envelope_model& _envelope(
		size_t azimuth, size_t src_beam, size_t rcv_beam )
	{
		return *_envelopes[azimuth][src_beam][rcv_beam];
	}

	/**
	 * Writes the envelope data to disk
	 */
	void write_netcdf(const char* filename) ;

private:

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

	/** Number of receiver azimuths in result. */
	size_t _num_azimuths;

	/** Number of source beams in result. */
	size_t _num_src_beams;

	/** Number of receiver beams in result. */
	size_t _num_rcv_beams;

	/**
	 * Reverberation envelopes for each combination of results.
	 * The order of indices is azimuth number, source beam number,
	 * and then receiver beam number.
	 */
	envelope_model**** _envelopes;
};

}   // end of namespace eigenverb
}   // end of namespace usml
