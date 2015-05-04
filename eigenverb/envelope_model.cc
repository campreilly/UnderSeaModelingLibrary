/**
 * @file envelope_model.cc
 * Reverberation envelope time series for a single combination of
 * receiver azimuth, source beam number, receiver beam number.
 */
#include <boost/foreach.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <usml/eigenverb/envelope_model.h>

using namespace usml::eigenverb;

#define DEBUG_ENVELOPE

/**
 * Reserve the memory used to store the results of this calculation.
 */
envelope_model::envelope_model(
	const seq_vector* transmit_freq,
	const seq_vector* travel_time,
	double pulse_length, double threshold
) :
	_transmit_freq(transmit_freq),
	_travel_time(travel_time->data()),
	_pulse_length( pulse_length ),
	_threshold( threshold * pulse_length ),
	_energy(transmit_freq->size()),
	_duration(transmit_freq->size()),
	_intensity(transmit_freq->size(), travel_time->size())
{
}

/**
 * Adds a single combination of source and receiver eigenverbs
 * to this time series.
 */
bool envelope_model::compute_intensity( const vector<double>& scatter,
		const eigenverb& src_verb, const eigenverb& rcv_verb )
{
	bool ok = compute_overlap( scatter, src_verb, rcv_verb  );
	if ( !ok ) return false ;

	compute_time_series( src_verb, rcv_verb ) ;
	return true ;
}

/**
 * Compute the total energy of the overlap between two eigenverbs.
 */
bool envelope_model::compute_overlap( const vector<double>& scatter,
		const eigenverb& src_verb, const eigenverb& rcv_verb )
{
	// determine the range and bearing between the projected Gaussians
	// normalize bearing to min distance between angles
	// exit early if source more than 5 beam width away

    double bearing ;
    const double range = rcv_verb.position.gc_range( src_verb.position, &bearing ) ;
    if ( range < 1e-6 ) bearing = 0 ;	// fixes bearing = NaN
    bearing -= rcv_verb.direction ;
    bearing = M_PI - abs( fmod(abs(bearing), TWO_PI) - M_PI);

    const double xs = range * cos( bearing ) ;
    const double xs2 = xs * xs ;
    if ( xs2 > 25.0 * rcv_verb.length2[0] ) return false ;

    const double ys = range * sin( bearing ) ;
    const double ys2 = ys * ys ;
    if ( ys2 > 25.0 * rcv_verb.width2[0] ) return false ;

	#ifdef DEBUG_ENVELOPE
    	cout << "*** envelope_model::compute_overlap ***" << endl ;
		cout << "    Travel time:     " << src_verb.time + rcv_verb.time << endl;
		cout << "        DE:          " << src_verb.source_de << endl;
		cout << "       range:        " << range << endl;
		cout << "       bearing:      " << bearing << endl;
		cout << "        xs:          " << xs << endl;
		cout << "        ys:          " << ys << endl;
		cout << "   grazing angle:    " << src_verb.grazing*180.0/M_PI << endl;
		cout << "      Loss in:       " << 10.0*log10(src_verb.energy) << endl;
		cout << "      Loss out:      " << 10.0*log10(rcv_verb.energy) << endl;
		cout << "     Two-way TL:     " << 10.0*log10(element_prod(src_verb.energy, rcv_verb.energy)) << endl;
		cout << "scattering strength: " << 10.0*log10(scatter) << endl;
	#endif

	// determine the relative tilt between the projected Gaussians
	// normalize tilt to min distance between angles

	double alpha = src_verb.direction - rcv_verb.direction;
	alpha = M_PI - abs( fmod(abs(alpha), TWO_PI) - M_PI);
	const double cos2alpha = cos(2.0 * alpha);
	const double sin2alpha = sin(2.0 * alpha);
//    cout << "alpha=" << alpha
// 		 << " cos2alpha=" << cos2alpha
//		 << " sin2alpha=" << sin2alpha << endl ;

    // compute the intersection of the Gaussian profiles

    vector<double> src_sum = src_verb.length2 + src_verb.width2 ;
    vector<double> src_diff = src_verb.length2 - src_verb.width2 ;
    vector<double> src_prod = src_verb.length2 * src_verb.width2 ;
//    cout << "src_sum=" << src_sum
// 		 << " src_diff=" << src_diff
//		 << " src_prod=" << src_prod << endl ;

    vector<double> rcv_sum = rcv_verb.length2 + rcv_verb.width2 ;
    vector<double> rcv_diff = rcv_verb.length2 - rcv_verb.width2 ;
    vector<double> rcv_prod = rcv_verb.length2 * rcv_verb.width2 ;
//    cout << "rcv_sum=" << src_sum
// 		 << " rcv_diff=" << src_diff
//		 << " rcv_prod=" << src_prod << endl ;

    // compute the scaling of the exponential
    // equations (26) and (28) from the paper

    vector<double> det_sr = 0.5 * ( 2.0 * ( src_prod + rcv_prod )
    		+ ( src_sum * rcv_sum ) - ( src_diff * rcv_diff ) * cos2alpha ) ;
    noalias(_energy) = element_div(
    		TWO_PI * src_verb.energy * rcv_verb.energy * scatter,
			sqrt( det_sr ) ) ;
//    cout << "det_sr=" << det_sr
// 		 << " energy=" << _energy << endl ;

    // check threshold to avoid calculations for weak signals

    bool ok = false ;
    BOOST_FOREACH( double level, _energy ) {
    	if ( level > _threshold ) {
    		ok = true ;
    		break ;
    	}
    }
    if ( !ok ) {
		#ifdef DEBUG_ENVELOPE
			cout << "      Energy:        " << 10.0*log10(_energy) << endl
				 << "*** threshold not met ***" << endl ;
		#endif
		return false ;
    }

    // compute the power of the exponential
    // equation (28) from the paper

    src_prod = src_diff * cos2alpha ;
    vector<double> kappa =
  		  xs2 * ( src_sum + src_prod * 2.0 * rcv_verb.length2 )
		+ ys2 * ( src_sum - src_prod + 2.0 * rcv_verb.width2 )
		- 2.0 * xs * ys * src_diff * sin2alpha ;
    _energy = element_prod( _energy, exp( -0.25*element_div(kappa,det_sr) ) ) ;
//    cout << "src_prod=" << src_prod
// 		 << " kappa=" << kappa
//		 << " energy=" << _energy << endl ;

    // check threshold again to avoid calculations for weak signals

    ok = false ;
    BOOST_FOREACH( double level, _energy ) {
    	if ( level > _threshold ) {
    		ok = true ;
    		break ;
    	}
    }
    if ( !ok ) {
		#ifdef DEBUG_ENVELOPE
			cout << "      Energy:        " << 10.0*log10(_energy) << endl
				 << "*** threshold not met ***" << endl ;
		#endif
		return false ;
    }

    // compute the duration of the overlap

    det_sr = element_div( det_sr, src_prod * rcv_prod ) ;
	src_sum = 1.0 / src_verb.width2 + 1.0 / src_verb.length2 ;
	src_diff = 1.0 / src_verb.width2 - 1.0 / src_verb.length2 ;
	noalias(_duration) = 0.5 * element_div(
			( 1.0 / src_verb.width2 + 1.0 / src_verb.length2 )
			+ ( 1.0 / src_verb.width2 - 1.0 / src_verb.length2 ) * cos2alpha
			+ 2.0 / rcv_verb.width2,
			det_sr ) ;
//    cout << "det_sr=" << det_sr
//    	 << " src_sum=" << src_sum
//	     << " src_diff=" << src_diff
//		 << " duration=" << _duration << endl ;

	double factor = cos( rcv_verb.grazing ) / rcv_verb.sound_speed ;
	_duration = 0.5 * sqrt( _pulse_length * _pulse_length
			+ factor * factor * (_duration) ) ;
//    cout << "factor=" << factor << endl
//		 << " duration=" << _duration << endl ;
	#ifdef DEBUG_ENVELOPE
		cout << "      Energy:        " << 10.0*log10(_energy) << endl;
		cout << "      Duration:        " << _duration << endl;
	#endif

    return true ;
}

/**
 * Computes Gaussian time series contribution given delay, duration, and
 * total energy.
 */
void envelope_model::compute_time_series(
	const eigenverb& src_verb, const eigenverb& rcv_verb )
{
	const double coeff = 1.0 / (4.0 * M_PI * sqrt(TWO_PI));
	for (size_t f = 0; f < _transmit_freq->size(); ++f) {
		double delay = src_verb.time + rcv_verb.time + _duration[f];
		double scale = _energy[f] * coeff / _duration[f];
		matrix_row<matrix<double> > row(_intensity, f);
		row = scale * exp(-0.5 * abs2((_travel_time - delay) / _duration[f]));
	}
}

