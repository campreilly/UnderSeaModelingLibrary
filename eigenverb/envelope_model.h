/**
 * @file envelope_model.h
 * Computes the reverberation envelope time series for a single combination of
 * receiver azimuth, source beam number, receiver beam number.
 */
#pragma once

#include <usml/types/seq_vector.h>
#include <usml/eigenverb/eigenverb.h>

namespace usml {
namespace eigenverb {

using namespace usml::types;

class envelope_collection ; // forward declaration

/**
 * Computes the reverberation envelope time series for a single combination of
 * receiver azimuth, source beam number, receiver beam number. The envelope is
 * stored as a matrix that represents the results a function of the
 * sensor_pair's envelope frequency (rows) and two-way travel time (columns).
 *
 * This implementation requires the receiver eigenverbs to be interpolated
 * onto the envelope frequencies.  However, to save time, it assumes that
 * the envelope frequencies are a subset of the source eigenverbs frequencies,
 * so that no interpolation is required.
 *
 * There are no public methods in this class.  It just acts as a set of service
 * routines for the envelope_collection class.
 *
 * @xref S. Reilly, D. Thibaudeau, T. Burns, "Fast computation of
 * reverberation using Gaussian beam reflections," Report to NAWCTSD,
 * October 20, 2014.
 */
class USML_DECLSPEC envelope_model {

friend class envelope_collection ;

private:

	/**
	 * Reserve the memory used to store the results of this calculation.
	 *
	 * @param envelope_freq		Frequencies at which the source and receiver
	 * 							eigenverbs overlap (Hz).  Frequencies at which
	 * 							envelope will be computed.
	 * @param src_freq_first	Index of the first source frequency that
	 * 							overlaps receiver (Hz).  Used to map
	 * 							source eigenverbs onto envelope_freq values.
	 * @param travel_time		Times at which the sensor_pair's
	 * 							reverberation envelopes are computed (Hz).
	 * @param pulse_length		Duration of the transmitted pulse (sec).
	 * 							Defines the temporal resolution of the envelope.
	 * @param threshold			Minimum intensity level for valid reverberation
	 * 							contributions (linear units).
	 */
	envelope_model(
		const seq_vector* envelope_freq,
		size_t src_freq_first,
		const seq_vector* travel_time,
		double pulse_length, double threshold
	) ;

	/**
	 * Default constructor.
	 */
	~envelope_model() ;

	/**
	 * Computes the intensity for a single combination of source and receiver
	 * eigenverbs.  Assumes that the source and receiver eigenverbs
	 * have been interpolated onto the sensor_pair's frequency domain before
	 * this routine is called. It also assumes that the calling routine
	 * has computed the scattering coefficient; which saves this
	 * class from having to know anything about the ocean.
	 *
	 * @param src_verb		Eigenverb contribution from the source
	 * 						at the original source frequencies.
	 * @param rcv_verb		Eigenverb contribution from the receiver
	 * 						interpolated onto the envelope frequencies.
	 * @param scatter		Scattering strength coefficient for this
	 * 						combination of eigenverbs (ratio).
	 * @param xs2			Square of the relative distance from the
	 * 						receiver to the target along the direction
	 * 						of the receiver's length.
	 * @param ys2			Square of the relative distance from the
	 * 						receiver to the target along the direction
	 * 						of the receiver's width.
	 * @param pulse_length	Duration of the transmit pulse (sec).
	 * @return				False if reverbereation energy below threshold.
	 */
	bool compute_intensity(
			const eigenverb& src_verb, const eigenverb& rcv_verb,
			const vector<double>& scatter, double xs2, double ys2 ) ;

	/**
	 * Reverberation intensity at each point the time series.
	 * Each row represents a specific envelope frequency.
	 * Each column represents a specific travel time.
	 * Passing this back as a non-const reference allows it to be accessed
	 * by a matrix_row<> proxy in the calling program.
	 */
	matrix< double >& intensity() {
		return _intensity ;
	}

private:

	/**
	 * Compute the total energy and duration of the overlap between
	 * two eigenverbs. Implements the analytic solution for energy of
	 * the bistatic reverberation contribution from eqn. (28) ans (29)
	 * in the paper.  Computes the duration from eqn. (45) and (33).
	 *
	 * @param src_verb		Eigenverb contribution from the source,
	 * 						at the original source frequencies.
	 * @param rcv_verb		Eigenverb contribution from the receiver,
	 * 						interpolated onto the envelope frequencies.
	 * @param scatter		Scattering strength coefficient for this
	 * 						combination of eigenverbs,
	 * 						as a function of envelope frequency (ratio).
	 * @param xs2			Square of the relative distance from the
	 * 						receiver to the target along the direction
	 * 						of the receiver's length.
	 * @param ys2			Square of the relative distance from the
	 * 						receiver to the target along the direction
	 * 						of the receiver's width.
	 * @return				False if energy below threshold.
	 */
	bool compute_overlap(
			const eigenverb& src_verb, const eigenverb& rcv_verb,
			const vector<double>& scatter, double xs2, double ys2 ) ;

	/**
	 * Computes Gaussian time series contribution given delay, duration, and
	 * total energy.  Implements equation (6) from the paper.  Replaces the
	 * values previously held by the _intensity member variable.
	 *
	 * In an effort to speed up the calculation of the Gaussian, this
	 * routine uses uBLAS vector and matrix proxies to only compute the
	 * portion of the time series within +/- five (5) times the duration
	 * of each pulse.
	 *
	 * @param src_verb		One way travel time for source eigenverb.
	 * @param rcv_verb		One way travel time for receiver eigenverb.
	 */
	void compute_time_series( double src_verb_time, double rcv_verb_time ) ;

	/**
	 * Frequencies at which the source and receiver eigenverbs overlap (Hz).
	 * Frequencies at which envelope will be computed.
	 */
	const seq_vector* _envelope_freq ;

	/**
	 * Index of the first source frequency that overlaps receiver (Hz).
	 * Used to map source eigenverbs onto envelope_freq values.
	 */
	const size_t _src_freq_first ;

	/**
	 * Times at which the sensor_pair's reverberation envelopes
	 * are computed (sec).  These times are not required to be evenly spaced.
	 * In the compute_time_series(), the find() method is used on this
	 * variable to search for the window in which to deposit the
	 * Gaussian contribution from each eigenverb overlap.
	 */
	seq_vector* _travel_time ;

	/**
	 * Duration of the transmitted pulse (sec).
	 * Defines the temporaal resolution of the envelope calculation.
	 */
	const double _pulse_length ;

	/**
	 * Minimum energy level for valid reverberation contributions
	 * (linear units).  Note that this is converted from intensity units
	 * in the constructor.
	 */
	const double _threshold ;

	/**
	 * Reverberation intensity at each point the time series.
	 * Each row represents a specific envelope frequency.
	 * Each column represents a specific travel time.
	 */
	matrix< double > _intensity;

	/**
	 * Workspace for storing total energy of eigenverb overlap,
	 * as a function of envelope frequency (linear units).
	 */
	vector<double> _energy ;

	/**
	 * Workspace for storing duration result of eigenverb overlap,
	 * as a function of envelope frequency (sec).
	 */
	vector<double> _duration ;

	/**
	 * Times in the form of a uBLAS vector.  This is used as the independent
	 * variable during compute_time_series().
	 * Making is a member variable prevents us from having to
	 * rebuilt it for each calculation.
	 */
	vector<double> _time_vector ;

	/**
	 * Workspace for storing intensity as a function of time (linear units).
	 * The Gaussian contributions from each eigenverb overlap are added
	 * to this object, and then this object is copied into the appropriate
	 * row of the _intensity object.
	 * Making is a member variable prevents us from having to
	 * rebuilt it for each calculation.
	 */
	vector<double> _level ;
};

}   // end of namespace eigenverb
}   // end of namespace usml
