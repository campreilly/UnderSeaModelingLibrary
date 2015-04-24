/**
 * @file envelope_spectrum.cc
 * Reverberation envelope spectrum for a single combination of
 * receiver azimuth, source beam number, receiver beam number.
 */
#include <boost/foreach.hpp>
#include <usml/eigenverb/envelope_model.h>

using namespace usml::eigenverb;

/**
 * Adds a single combination of source and receiver eigenverbs
 * to this spectrum.
 */
void envelope_model::add_contribution(
		const eigenverb& src_verb, const eigenverb& rcv_verb,
		const vector<double>& scatter, double pulse_length )
{
	vector<double> energy, duration ;
	bool ok = compute_overlap(
			src_verb, rcv_verb, scatter, pulse_length,
			&energy, &duration ) ;
	if ( ok ) {
		compute_spectrum(src_verb, rcv_verb, energy, duration ) ;
	}
}

/**
 * Compute the total energy of the overlap between two eigenverbs.
 */
bool envelope_model::compute_overlap(
		const eigenverb& src_verb, const eigenverb& rcv_verb,
		const vector<double>& scatter, double pulse_length,
		vector<double>* energy, vector<double>* duration )
{
	#ifdef DEBUG_CONVOLUTION
		cout << "*****envelope_spectrum::compute_overlap*****" << endl;
		cout << "    Travel time:     " << src_verb.travel_time + rcv_verb.travel_time << endl;
		cout << "        DE:          " << src_verb.launch_de << endl;
		cout << "     Path length:    " << src_verb.distance << endl;
		cout << "       range:        " << range << endl;
		cout << "        xs:          " << xs << endl;
		cout << "        ys:          " << ys << endl;
		cout << "       Area:         " << _area << endl;
		cout << "   grazing angle:    " << src_verb.grazing*180.0/M_PI << endl;
		cout << "      Loss in:       " << 10.0*log10(src_verb.intensity) << endl;
		cout << "      Loss out:      " << 10.0*log10(rcv_verb.intensity) << endl;
		cout << "     Two-way TL:     " << 10.0*log10(element_prod(src_verb.intensity, rcv_verb.intensity)) << endl;
		cout << "scattering strength: " << 10.0*log10(scatter) << endl;
	#endif

	// determine the range and bearing between the projected Gaussians
	// normalize bearing to min distance between angles

    double bearing ;
    double range = rcv_verb.position.gc_range( src_verb.position, &bearing ) ;
    bearing -= rcv_verb.direction ;
    bearing = M_PI - abs( fmod(abs(bearing), TWO_PI) - M_PI);
    double xs = range * cos( bearing ) ;
    double ys = range * sin( bearing ) ;

	// determine the relative tilt between the projected Gaussians
	// normalize tilt to min distance between angles

    double alpha = src_verb.direction - rcv_verb.direction;
	alpha = M_PI - abs( fmod(abs(alpha), TWO_PI) - M_PI);
	double cos2alpha = cos(2.0 * alpha);
	double sin2alpha = sin(2.0 * alpha);

    // compute the intersection of the Gaussian profiles

    vector<double> src_sum = src_verb.length2 + src_verb.width2 ;
    vector<double> src_diff = src_verb.length2 - src_verb.width2 ;
    vector<double> src_prod = src_verb.length2 * src_verb.width2 ;

    vector<double> rcv_sum = rcv_verb.length2 + rcv_verb.width2 ;
    vector<double> rcv_diff = rcv_verb.length2 - rcv_verb.width2 ;
    vector<double> rcv_prod = rcv_verb.length2 * rcv_verb.width2 ;

    // compute the scaling of the exponential
    // equations (26) and (28) from the paper

    vector<double> det_sr = 0.5 * ( 2.0 * ( src_prod + rcv_prod )
    		+ ( src_sum * rcv_sum ) - ( src_diff * rcv_diff ) * cos2alpha ) ;
    *energy = element_div(
    		TWO_PI * src_verb.energy * rcv_verb.energy * scatter,
			sqrt( det_sr ) ) ;

    // check threshold to avoid calculations for weak signals

    bool ok = false ;
    BOOST_FOREACH( double level, *energy ) {
    	if ( level > _threshold ) {
    		ok = true ;
    		break ;
    	}
    }
    if ( !ok ) {
		#ifdef DEBUG_CONVOLUTION
			cout << "      Energy:        " << 10.0*log10(*energy) << endl
				 << "*** threshold not met ***" << endl ;
		#endif
		return false ;
    }

    // compute the power of the exponential
    // equation (28) from the paper

    src_prod = src_diff * cos2alpha ;
    vector<double> kappa =
  		  xs*xs * ( src_sum + src_prod * 2.0 * rcv_verb.length2 )
		+ ys*ys * ( src_sum - src_prod + 2.0 * rcv_verb.width2 )
		- 2.0 * xs * ys * src_diff * sin2alpha ;
    *energy = element_prod( *energy, exp( -0.25*element_div(kappa,det_sr) ) ) ;

    // check threshold again to avoid calculations for weak signals

    ok = false ;
    BOOST_FOREACH( double level, *energy ) {
    	if ( level > _threshold ) {
    		ok = true ;
    		break ;
    	}
    }
    if ( !ok ) {
		#ifdef DEBUG_CONVOLUTION
			cout << "      Energy:        " << 10.0*log10(*energy) << endl
				 << "*** threshold not met ***" << endl ;
		#endif
		return false ;
    }

    // compute the duration of the overlap

    det_sr = element_div( det_sr, src_prod * rcv_prod ) ;
	src_sum = 1.0 / src_verb.width2 + 1.0 / src_verb.length2 ;
	src_diff = 1.0 / src_verb.width2 - 1.0 / src_verb.length2 ;
	*duration = 0.5 * element_div(
			( 1.0 / src_verb.width2 + 1.0 / src_verb.length2 )
			+ ( 1.0 / src_verb.width2 - 1.0 / src_verb.length2 ) * cos2alpha
			+ 2.0 / rcv_verb.width2,
			det_sr ) ;

	double factor = cos( rcv_verb.grazing ) / rcv_verb.sound_speed ;
	*duration = 0.5 * sqrt( pulse_length*pulse_length
			+ factor * factor * (*duration) ) ;
	#ifdef DEBUG_CONVOLUTION
		cout << "      Energy:        " << 10.0*log10(*energy) << endl;
		cout << "      Duration:        " << *duration << endl;
	#endif

    return true ;
}

/**
 * Adds this contribution to the reverberation envelope spectrum.
 *
 * TODO add Doppler shift to kappa
 * TODO check the sign on the phase delay
 */
void envelope_model::compute_spectrum(
		const eigenverb& src_verb, const eigenverb& rcv_verb,
		const vector<double>& energy, const vector<double>& duration )
{
	const double coeff = 1.0 / ( 4.0 * M_PI * sqrt(TWO_PI) ) ;
	vector<double> delay = src_verb.time + rcv_verb.time + duration ;
	vector<double> bandwidth = 1.0  / duration ;

	for ( size_t s=0 ; s < _source_freq->size() ; ++s ) {
		for ( size_t e=0 ; e < _envelope_freq->size() ; ++e ) {
			const double freq = (*_envelope_freq)[e] ;
			const double kappa = freq / bandwidth[s] ;
			complex<double> value(
				-0.5 * kappa * kappa,
				-TWO_PI * freq * delay[s] );
			_pressure(s,e) += value * energy[s] / bandwidth[s] * coeff ;
		}
	}
}

