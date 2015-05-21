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

#define DEBUG_ENVELOPE
#define ENVELOPE_WINDOW

/**
 * Reserve the memory used to store the results of this calculation.
 */
envelope_model::envelope_model(
	const seq_vector* envelope_freq,
	size_t src_freq_first,
	const seq_vector* travel_time,
	double pulse_length, double threshold
) :
	_envelope_freq(envelope_freq),
	_src_freq_first(src_freq_first),
	_travel_time( travel_time->clone() ),
	_pulse_length( pulse_length ),
	_threshold( threshold * pulse_length ),
	_intensity(envelope_freq->size(), travel_time->size()),
	_energy(envelope_freq->size()),
	_duration(envelope_freq->size()),
	_level(travel_time->size())
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
 * Compute the total energy of the overlap between two eigenverbs.
 */
bool envelope_model::compute_overlap(
	const eigenverb& src_verb, const eigenverb& rcv_verb,
	const vector<double>& scatter, double xs2, double ys2 )
{
	#ifdef DEBUG_ENVELOPE
    	cout << "*** envelope_model::compute_overlap ***" << endl ;
		cout << "    Travel time:     " << src_verb.time + rcv_verb.time << endl;
		cout << "        DE:          " << src_verb.source_de << endl;
		cout << "   grazing angle:    " << src_verb.grazing*180.0/M_PI << endl;
		cout << "      Loss in:       " << 10.0*log10(src_verb.energy) << endl;
		cout << "      Loss out:      " << 10.0*log10(rcv_verb.energy) << endl;
		// Make sure their the same size - else crash
		if (src_verb.energy.size() == rcv_verb.energy.size()) {
		    cout << "     Two-way TL:     " <<
		     10.0*log10(element_prod(src_verb.energy, rcv_verb.energy)) << endl;
		}
		cout << "scattering strength: " << 10.0*log10(scatter) << endl;
	#endif

    #ifdef DEBUG_ENVELOPE
        cout << "*** envelope_model:: src_verb ***" << endl ;

        cout << "    src_verb.time:        " << src_verb.time << endl;
        cout << "    src_verb.freq         " << *src_verb.frequencies << endl;
        cout << "    src_verb.energy       " << src_verb.energy << endl;
        cout << "    src_verb.length2      " << src_verb.length2 << endl;
        cout << "    src_verb.width2       " << src_verb.width2 << endl;
        cout << "    src_verb.latitude     " << src_verb.position.latitude() << endl;
        cout << "    src_verb.longitude    " << src_verb.position.longitude() << endl;
        cout << "    src_verb.altitude     " << src_verb.position.altitude() << endl;
        cout << "    src_verb.direction    " << src_verb.direction << endl;
        cout << "    src_verb.grazing      " << src_verb.grazing << endl;
        cout << "    src_verb.sound_speed  " << src_verb.sound_speed << endl;
        cout << "    src_verb.de_index     " << src_verb.de_index << endl;
        cout << "    src_verb.az_index     " << src_verb.az_index << endl;
        cout << "    src_verb.source_de    " << src_verb.source_de << endl;
        cout << "    src_verb.source_az    " << src_verb.source_az << endl;
        cout << "    src_verb.surface      " << src_verb.surface << endl;
        cout << "    src_verb.bottom       " << src_verb.bottom << endl;
        cout << "    src_verb.caustic      " << src_verb.caustic << endl;
        cout << "    src_verb.upper        " << src_verb.upper << endl;
        cout << "    src_verb.lower        " << src_verb.lower << endl;

        cout << "*** envelope_model:: rcv_verb ***" << endl ;

        cout << "    rcv_verb.time:        " << rcv_verb.time << endl;
        cout << "    rcv_verb.freq         " << *rcv_verb.frequencies << endl;
        cout << "    rcv_verb.energy       " << rcv_verb.energy << endl;
        cout << "    rcv_verb.length2      " << rcv_verb.length2 << endl;
        cout << "    rcv_verb.width2       " << rcv_verb.width2 << endl;
        cout << "    rcv_verb.latitude     " << rcv_verb.position.latitude() << endl;
        cout << "    rcv_verb.longitude    " << rcv_verb.position.longitude() << endl;
        cout << "    rcv_verb.altitude     " << rcv_verb.position.altitude() << endl;
        cout << "    rcv_verb.direction    " << rcv_verb.direction << endl;
        cout << "    rcv_verb.grazing      " << rcv_verb.grazing << endl;
        cout << "    rcv_verb.sound_speed  " << rcv_verb.sound_speed << endl;
        cout << "    rcv_verb.de_index     " << rcv_verb.de_index << endl;
        cout << "    rcv_verb.az_index     " << rcv_verb.az_index << endl;
        cout << "    rcv_verb.source_de    " << rcv_verb.source_de << endl;
        cout << "    rcv_verb.source_az    " << rcv_verb.source_az << endl;
        cout << "    rcv_verb.surface      " << rcv_verb.surface << endl;
        cout << "    rcv_verb.bottom       " << rcv_verb.bottom << endl;
        cout << "    rcv_verb.caustic      " << rcv_verb.caustic << endl;
        cout << "    rcv_verb.upper        " << rcv_verb.upper << endl;
        cout << "    rcv_verb.lower        " << rcv_verb.lower << endl;

    #endif

	// determine the relative tilt between the projected Gaussians
	// normalize tilt to min distance between angles

	double alpha = src_verb.direction - rcv_verb.direction;
	alpha = M_PI - abs( fmod(abs(alpha), TWO_PI) - M_PI);
	const double cos2alpha = cos(2.0 * alpha);
	const double sin2alpha = sin(2.0 * alpha);

	// TODO Remove debugging output
	cout << "alpha=" << alpha
 		 << " cos2alpha=" << cos2alpha
		 << " sin2alpha=" << sin2alpha << endl ;

	// define subset of frequency dependent terms in source
    //
    // Although the use of const_cast<> allows us to ignore the read-only
    // nature of src_verb, we are *very careful* to not write anything to it.

	range window( _src_freq_first, _src_freq_first + _envelope_freq->size() ) ;
	eigenverb& verb = const_cast<eigenverb&>( src_verb ) ;
	const vector_range< vector<double> > src_verb_length2( verb.length2, window ) ;
	const vector_range< vector<double> > src_verb_width2( verb.width2, window ) ;
	const vector_range< vector<double> > src_verb_energy( verb.energy, window ) ;

    // compute the intersection of the Gaussian profiles

    vector<double> src_sum = src_verb_length2 + src_verb_width2 ;
    vector<double> src_diff = src_verb_length2 - src_verb_width2 ;
    vector<double> src_prod = src_verb_length2 * src_verb_width2 ;

    // TODO Remove debugging output
    cout << "src_sum=" << src_sum
 		 << " src_diff=" << src_diff
		 << " src_prod=" << src_prod << endl ;

    vector<double> rcv_sum = rcv_verb.length2 + rcv_verb.width2 ;
    vector<double> rcv_diff = rcv_verb.length2 - rcv_verb.width2 ;
    vector<double> rcv_prod = rcv_verb.length2 * rcv_verb.width2 ;

    // TODO Remove debugging output
    cout << "rcv_sum=" << src_sum
 		 << " rcv_diff=" << src_diff
		 << " rcv_prod=" << src_prod << endl ;

    // compute the scaling of the exponential
    // equations (26) and (28) from the paper

    vector<double> det_sr = 0.5 * ( 2.0 * ( src_prod + rcv_prod )
    		+ ( src_sum * rcv_sum ) - ( src_diff * rcv_diff ) * cos2alpha ) ;
    noalias(_energy) = element_div(
    		TWO_PI * src_verb_energy * rcv_verb.energy * scatter,
			sqrt( det_sr ) ) ;
    cout << "det_sr=" << det_sr
 		 << " energy=" << _energy << endl ;

    // TODO Remove debugging
    BOOST_FOREACH( double level, _energy ) {
        if ( level > 1.0 ) {
            cout << " Bad Energy Level" << endl;
        }
    }

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
//            vector<double> energy;
//            BOOST_FOREACH( double level, _energy ) {
//                double new_level = max(1e-30, level);
//                energy
//            }
//			cout << "      Energy:        " << 10.0*log10(energy) << endl
            cout    << "*** threshold not met ***" << endl ;
		#endif
		return false ;
    }

    // compute the power of the exponential
    // equation (28) from the paper

    src_prod = src_diff * cos2alpha ;
    vector<double> kappa =
  		  xs2 * ( src_sum + src_prod * 2.0 * rcv_verb.length2 )
		+ ys2 * ( src_sum - src_prod + 2.0 * rcv_verb.width2 )
		- 2.0 * sqrt( xs2 * ys2 ) * src_diff * sin2alpha ;

    // TODO Remove after debugged
    cout << "src_prod=" << src_prod
         << " kappa=" << kappa << endl;
    BOOST_FOREACH( double value, kappa ) {
        if (value < 0.0) {
             cout    << "*** Kappa less than zero ***" << endl ;
            return false;
        }
    }


    _energy = element_prod( _energy, exp( -0.25*element_div(kappa,det_sr) ) ) ;

    cout << " energy = " << _energy << endl ;

    // TODO Remove debugging
    BOOST_FOREACH( double level, _energy ) {
        if ( level > 1.0 ) {
            cout << " Bad Energy Level" << endl;
        }
    }

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
          //  cout << "      Energy:        " << 10.0*log10(max(1e-30,_energy)) << endl
             cout    << "*** threshold not met ***" << endl ;
		#endif
		return false ;
    }

    // compute the duration of the overlap
    // equation (41) from the paper

    det_sr = element_div( det_sr, src_prod * rcv_prod ) ;
	src_sum = 1.0 / src_verb_width2 + 1.0 / src_verb_length2 ;
	src_diff = 1.0 / src_verb_width2 - 1.0 / src_verb_length2 ;
	noalias(_duration) = 0.5 * element_div(
			( 1.0 / src_verb_width2 + 1.0 / src_verb_length2 )
			+ ( 1.0 / src_verb_width2 - 1.0 / src_verb_length2 ) * cos2alpha
			+ 2.0 / rcv_verb.width2,
			det_sr ) ;
	// TODO Remove debugging output
//    cout << "det_sr=" << det_sr
//    	 << " src_sum=" << src_sum
//	     << " src_diff=" << src_diff
//		 << " duration=" << _duration << endl ;

	// equation (33) from the paper
	double factor = cos( rcv_verb.grazing ) / rcv_verb.sound_speed ;
	_duration = 0.5 * sqrt( _pulse_length * _pulse_length
			+ factor * factor * (_duration) ) ;
	// TODO Remove debugging output
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
		double src_verb_time, double rcv_verb_time )
{
	const double coeff = 1.0 / (4.0 * M_PI * sqrt(TWO_PI));
	_intensity.clear() ;

	for ( size_t f = 0 ;f < _envelope_freq->size(); ++f ) {

		// compute the peak time and intensity

		double delay = src_verb_time + rcv_verb_time + _duration[f];
		double scale = _energy[f] * coeff / _duration[f];

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
					(*_travel_time - delay) / _duration[f]));
//			cout << "f=" << f << " " << intensity << endl ;
		#endif
	}
}

