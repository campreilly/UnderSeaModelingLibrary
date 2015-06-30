/**
 * @file envelope_model.cc
 * Reverberation envelope time series for a single combination of
 * receiver azimuth, source beam number, receiver beam number.
 */
#include <boost/foreach.hpp>
#include <boost/numeric/ublas/vector_proxy.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <usml/eigenverb/envelope_model.h>

using namespace usml::eigenverb;

//#define DEBUG_ENVELOPE
//#define ENVELOPE_WINDOW

/**
 * Reserve the memory used to store the results of this calculation.
 */
envelope_model::envelope_model(
	const seq_vector* envelope_freq,
	size_t src_freq_first,
	const seq_vector* travel_time,
	double initial_time,
	double pulse_length,
	double threshold
) :
	_envelope_freq(envelope_freq),
	_src_freq_first(src_freq_first),
	_travel_time( travel_time->clone() ),
	_initial_time(initial_time),
	_pulse_length(pulse_length),
	_threshold(threshold),
	_power(envelope_freq->size()),
	_duration(envelope_freq->size()),
	_intensity(envelope_freq->size(), travel_time->size())
{
}

/**
 * Reserve the memory used to store the results of this calculation.
 */
envelope_model::~envelope_model() {
	delete _travel_time ;
}

/**
 * Adds a single combination of source and receiver eigenverbs
 * to this time series.
 */
bool envelope_model::compute_intensity(
		const eigenverb& src_verb, const eigenverb& rcv_verb,
		const vector<double>& scatter, double xs2, double ys2 )
{
	bool ok = compute_overlap( src_verb, rcv_verb, scatter, xs2, ys2 );
	if ( !ok ) return false ;

	compute_time_series( src_verb.time, rcv_verb.time ) ;
	return true ;
}

/**
 * Compute the total power of the overlap between two eigenverbs.
 */
bool envelope_model::compute_overlap(
	const eigenverb& src_verb, const eigenverb& rcv_verb,
	const vector<double>& scatter, double xs2, double ys2 )
{
	#ifdef DEBUG_ENVELOPE
		cout << "wave_queue::compute_overlap() " << endl
			<< "\txs2=" << xs2
			<< " ys2=" << ys2
			<< " scatter=" << scatter << endl
			<< "\tsrc_verb"
			<< " t=" << src_verb.time
			<< " de=" << to_degrees(src_verb.source_de)
			<< " az=" << to_degrees(src_verb.source_az)
			<< " direction=" << to_degrees(src_verb.direction)
			<< " grazing=" << to_degrees(src_verb.grazing) << endl
			<< "\tpower=" << 10.0 * log10(src_verb.power)
			<< " length=" << sqrt(src_verb.length2)
			<< " width=" << sqrt(src_verb.width2) << endl
			<< "\tsurface=" << src_verb.surface << " bottom=" << src_verb.bottom
			<< " caustic=" << src_verb.caustic << endl
			<< "\trcv_verb"
			<< " t=" << rcv_verb.time
			<< " de=" << to_degrees(rcv_verb.source_de)
			<< " az=" << to_degrees(rcv_verb.source_az)
			<< " direction=" << to_degrees(rcv_verb.direction)
			<< " grazing=" << to_degrees(rcv_verb.grazing) << endl
			<< "\tpower=" << 10.0 * log10(rcv_verb.power)
			<< " length=" << sqrt(rcv_verb.length2)
			<< " width=" << sqrt(rcv_verb.width2) << endl
			<< "\tsurface=" << rcv_verb.surface << " bottom=" << rcv_verb.bottom
			<< " caustic=" << rcv_verb.caustic << endl;
	#endif

	// determine the relative tilt between the projected Gaussians

	const double alpha = src_verb.direction - rcv_verb.direction;
	const double cos2alpha = cos(2.0 * alpha);
	const double sin2alpha = sin(2.0 * alpha);

	// define subset of frequency dependent terms in source
    //
    // Although the use of const_cast<> allows us to ignore the read-only
    // nature of src_verb, we are *very careful* to not write anything to it.

	range window( _src_freq_first, _src_freq_first + _envelope_freq->size() ) ;
	eigenverb& verb = const_cast<eigenverb&>( src_verb ) ;
	const vector_range< vector<double> > src_verb_power( verb.power, window ) ;

    // compute commonly used terms in the intersection of the Gaussian profiles

	const double src_sum = src_verb.length2 + src_verb.width2 ;
	const double src_diff = src_verb.length2 - src_verb.width2 ;
	const double src_prod = src_verb.length2 * src_verb.width2 ;

	const double rcv_sum = rcv_verb.length2 + rcv_verb.width2 ;
	const double rcv_diff = rcv_verb.length2 - rcv_verb.width2 ;
	const double rcv_prod = rcv_verb.length2 * rcv_verb.width2 ;

    // compute the scaling of the exponential
    // equations (26) and (28) from the paper
	// TODO find source of mystery scaling factors (0.25 * 0.5)

    double det_sr = 0.5 * ( 2.0 * ( src_prod + rcv_prod )
    		+ ( src_sum * rcv_sum ) - ( src_diff * rcv_diff ) * cos2alpha ) ;
    noalias(_power) = 0.25 * 0.5 * _pulse_length
    		* src_verb_power * rcv_verb.power * scatter ;

    // compute the power of the exponential
    // equation (28) from the paper

    const double new_prod = src_diff * cos2alpha ;
    const double kappa = -0.25 * (
  		  xs2 * ( src_sum + new_prod + 2.0 * rcv_verb.length2 )
		+ ys2 * ( src_sum - new_prod + 2.0 * rcv_verb.width2 )
		- 2.0 * sqrt( xs2 * ys2 ) * src_diff * sin2alpha )
		/ det_sr ;
	#ifdef DEBUG_ENVELOPE
		cout << "\tsrc_verb_power=" << src_verb_power
			 << " rcv_verb.power=" << rcv_verb.power << endl
			 << "\tdet_sr=" << det_sr
			 << " kappa=" << kappa
			 << " power=" << (10.0*log10(_power)) << endl ;
	#endif
	_power *= exp( kappa ) / sqrt( det_sr ) ;

    // compute the square of the duration of the overlap
    // equation (41) from the paper

    det_sr = det_sr / ( src_prod * rcv_prod ) ;
	_duration = 0.5 * (
			( 1.0 / src_verb.width2 + 1.0 / src_verb.length2 )
			+ ( 1.0 / src_verb.width2 - 1.0 / src_verb.length2 ) * cos2alpha
			+ 2.0 / rcv_verb.width2
			) / det_sr ;

	// combine duration of the overlap with pulse length
	// equation (33) from the paper

	const double factor = cos( rcv_verb.grazing ) / rcv_verb.sound_speed ;
	_duration = 0.5 * sqrt( _pulse_length * _pulse_length
			+ factor * factor * _duration ) ;
	#ifdef DEBUG_ENVELOPE
		cout << "\tcontribution"
			<< " duration=" << _duration
			<< " power=" << (10.0*log10(_power)) << endl ;
	#endif

	// check threshold to avoid calculations for weak signals

	BOOST_FOREACH( double level, _power ) {
		if ( level / _duration > _threshold ) return true ;
	}
	return false ;
}

/**
 * Computes Gaussian time series contribution given delay, duration, and
 * total power.
 */
void envelope_model::compute_time_series(
		double src_verb_time, double rcv_verb_time )
{
	_intensity.clear() ;

	for ( size_t f = 0 ;f < _envelope_freq->size(); ++f ) {

		// compute the peak time and intensity

		double delay = src_verb_time + rcv_verb_time + _duration;
		double scale = _power[f] / _duration;

		// compute Gaussian intensity as a function of time
		//
		// TODO Test to see if using uBLAS vector proxies to limit computation is faster (issue #189)

		matrix_row< matrix<double> > intensity( _intensity, f ) ;

		#ifdef ENVELOPE_WINDOW

			// use uBLAS vector proxies to only compute the portion of the
			// time series within +/- five (5) times the duration

			_level.clear() ;
			size_t first = _travel_time->find_index(delay - 5.0 * _duration[f]);
			size_t last = _travel_time->find_index(delay + 5.0 * _duration[f]) + 1;
			range window(first, last);
			vector_range< seq_vector > time(*_travel_time, window);
			vector_range< vector<double> > level(_level, window);
			level = scale * exp(-0.5 * abs2((time - delay) / _duration[f]));
			intensity = _level ;

		#else

			// compute intensity at all times

			intensity = scale * exp(-0.5 * abs2(
					(*_travel_time + _initial_time - delay) / _duration));
		#endif
	}
}
