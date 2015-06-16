/**
 * @file envelope_generator.cc
 * Computes reverberation envelopes from eigenverbs.
 */
#include <usml/eigenverb/envelope_generator.h>
#include <usml/eigenverb/eigenverb.h>
#include <usml/sensors/beam_pattern_map.h>
#include <usml/sensors/beam_pattern_model.h>
#include <usml/threads/smart_ptr.h>
#include <boost/foreach.hpp>

using namespace usml::eigenverb ;
using namespace usml::sensors ;

/**
 * Minimum intensity level for valid reverberation contributions (dB).
 */
double envelope_generator::intensity_threshold = -300.0 ;

/**
 * The _mutex for the singleton sensor_manager.
 */
read_write_lock envelope_generator::_property_mutex;

/**
 * Time axis for reverberation calculation.  Defaults to
 * a linear sequence out to 400 sec with a sampling period of 0.1 sec.
 */
unique_ptr<const seq_vector> envelope_generator::_travel_time( new seq_linear(0.0,0.1,400.0) ) ;

/**
 * Copies envelope computation parameters from static memory into
 * this specific task.
 */
envelope_generator::envelope_generator(
	sensor_pair* sensor_pair,
	double initial_time,
	size_t src_freq_first,
	size_t num_azimuths
):
    _done(false),
    _initial_time(initial_time),
    _ocean( ocean_shared::current() ),
    _sensor_pair(sensor_pair),
    _src_eigenverbs(sensor_pair->source()->eigenverbs()),
    _rcv_eigenverbs(sensor_pair->receiver()->eigenverbs()),
    _eigenverb_interpolator(sensor_pair->receiver()->frequencies(),sensor_pair->frequencies())
{
    write_lock_guard guard(_property_mutex);

    // Get source_params for reverb_duration and pulse_length, and beam_list size
    source_params::reference src_params = _sensor_pair->source()->source();
    _src_beam_list = src_params->beam_list();

    // Get receiver_params for beam_list size
    sensor_params::id_type rcv_params_ID = _sensor_pair->receiver()->paramsID();
    receiver_params::reference rcv_params =
        receiver_params_map::instance()->find(rcv_params_ID);

    _rcv_beam_list = rcv_params->beam_list();

    add_envelope_listener(_sensor_pair);

    _envelopes = envelope_collection::reference( new envelope_collection(
    	_sensor_pair->frequencies(),
        src_freq_first,
        _travel_time.get(),
        src_params->reverb_duration(),
        src_params->pulse_length(),
        pow(10.0,intensity_threshold/10.0),
        num_azimuths,
        _src_beam_list.size(),
        _rcv_beam_list.size(),
        _initial_time,
        _sensor_pair->source()->sensorID(),
        _sensor_pair->receiver()->sensorID(),
        _sensor_pair->source()->position(),
        _sensor_pair->receiver()->position() ) ) ;
}

/**
 * Copies envelope computation parameters from static memory into
 * this specific task.
 */
envelope_generator::envelope_generator(
        const seq_vector* envelope_freq,
        size_t src_freq_first,
        const seq_vector* receiver_freq,
        double reverb_duration,
        double pulse_length,
        size_t num_azimuths,
        size_t num_src_beams,
        size_t num_rcv_beams,
        eigenverb_collection::reference src_eigenverbs,
        eigenverb_collection::reference rcv_eigenverbs
):
    _done(false),
    _initial_time(0.0),
    _ocean( ocean_shared::current() ),
    _sensor_pair(NULL),
    _src_eigenverbs(src_eigenverbs),
    _rcv_eigenverbs(rcv_eigenverbs),
    _eigenverb_interpolator(receiver_freq,envelope_freq)
{
    write_lock_guard guard(_property_mutex);

    _envelopes = envelope_collection::reference( new envelope_collection(
        envelope_freq,
        src_freq_first,
        _travel_time.get(),
        reverb_duration,
        pulse_length,
        pow(10.0,intensity_threshold/10.0),
        num_azimuths,
        num_src_beams,
        num_rcv_beams,
        0.0, // initial_time
        1, // Place holder for testing
        1,
        wposition1(0.0,0.0),
        wposition1(0.0,0.0) ) ) ; // Place holder for testing
}

/**
 * Executes the Eigenverb reverberation model.
 */
void envelope_generator::run() {

	// create memory for work products

    const seq_vector* freq = _envelopes->envelope_freq() ;
    size_t num_freq = freq->size() ;
    size_t num_src_beams = _envelopes->num_src_beams() ;
    size_t num_rcv_beams = _envelopes->num_rcv_beams() ;

	vector<double> scatter( num_freq ) ;
	matrix<double> src_beam( num_freq, num_src_beams ) ;
	matrix<double> rcv_beam( num_freq, num_rcv_beams ) ;

	eigenverb rcv_verb ;
	rcv_verb.frequencies = freq ;
	rcv_verb.energy = vector<double>( num_freq ) ;
	rcv_verb.length2 = vector<double>( num_freq ) ;
	rcv_verb.width2 = vector<double>( num_freq ) ;

	// loop through eigenrays for each interface

	for ( size_t interface=0 ; interface < _rcv_eigenverbs->num_interfaces() ; ++interface) {

#ifdef USML_DEBUG
       
        // record rcv eigenverbs for each interface to their own disk file
        const char* rcv_ncname = "rcv_eigenverbs_";
        std::ostringstream rcv_filename;
        rcv_filename << rcv_ncname << interface << ".nc";
        _rcv_eigenverbs->write_netcdf(rcv_filename.str().c_str(), interface);

        // record src eigenverbs for each interface to their own disk file
        const char* src_ncname = "src_eigenverbs_";
        std::ostringstream src_filename ;
        src_filename << src_ncname << interface << ".nc" ;
        _src_eigenverbs->write_netcdf( src_filename.str().c_str(),interface) ;

#endif
		BOOST_FOREACH( eigenverb verb, _rcv_eigenverbs->eigenverbs(interface) ) {
			_eigenverb_interpolator.interpolate(verb,&rcv_verb) ;
			BOOST_FOREACH( eigenverb src_verb, _src_eigenverbs->eigenverbs(interface) ) {

				// determine the range and bearing between the projected Gaussians
				// normalize bearing to min distance between angles
				// skip this combo if source peak more than 3 beam width away
				//
				// TODO Use quadtree to make searching through source
				//      eigenverbs faster.

			    double bearing ;
			    const double range = rcv_verb.position.gc_range( src_verb.position, &bearing ) ;
			    if ( range*range > 9.0 * max(rcv_verb.length2[0],rcv_verb.width2[0])) continue ;

			    if ( range < 1e-6 ) bearing = 0 ;	// fixes bearing = NaN
			    bearing -= rcv_verb.direction ;
			    bearing = M_PI - abs( fmod(abs(bearing), TWO_PI) - M_PI);

			    const double xs = range * cos( bearing ) ;
			    const double xs2 = xs * xs ;
			    if ( xs2 > 9.0 * rcv_verb.length2[0] ) continue ;

			    const double ys = range * sin( bearing ) ;
			    const double ys2 = ys * ys ;
			    if ( ys2 > 9.0 * rcv_verb.width2[0] ) continue ;

				// compute interface scattering strength
			    // continue if does not meet threshold
				if ( scattering( interface,
					 rcv_verb.position, *freq,
					 src_verb.grazing, rcv_verb.grazing,
					 src_verb.direction, rcv_verb.direction,
					 &scatter )  ) {

                    // compute beam levels

                    src_beam = beam_gain(_src_beam_list, freq, src_verb.source_de,
                        src_verb.source_az, _sensor_pair->source()->orient());
                    rcv_beam = beam_gain(_rcv_beam_list, freq, rcv_verb.source_de,
                        rcv_verb.source_az, _sensor_pair->receiver()->orient());

                    // create envelope contribution

                    _envelopes->add_contribution( src_verb, rcv_verb,
                            src_beam, rcv_beam, scatter, xs2, ys2 ) ;
				}
			}
		}
	}
	this->notify_envelope_listeners(_envelopes) ;
}

/**
 * Computes the beam_gain
 */
matrix<double> envelope_generator::beam_gain(
    sensor_params::beam_pattern_list beam_list,
    const seq_vector* freq, double de_rad, double az_rad, orientation orient)
{
    // beam_level requires ublas vector
    const vector<double> frequencies = *freq;

    matrix<double> beam_matrix( freq->size(), beam_list.size() ) ;

    vector<double> level( freq->size(), 0.0 ) ;

    BOOST_FOREACH( beam_pattern_model::id_type id, beam_list) {

        beam_pattern_model::reference bp = beam_pattern_map::instance()->find(id);
        bp->beam_level(de_rad, az_rad, orient, frequencies, &level ) ;

        std::copy(level.begin(), level.end(), beam_matrix.begin1());
    }

    // TODO Remove debugging output
#ifdef USML_DEBUG

    typedef boost::numeric::ublas::matrix<double> matrix;

    for(matrix::iterator1 it1 = beam_matrix.begin1(); it1 != beam_matrix.end1(); ++it1) {
        for(matrix::iterator2 it2 = it1.begin(); it2 !=it1.end(); ++it2) {
            std::cout << "(" << it2.index1() << "," << it2.index2() << ") = " << *it2;
        }
        cout << endl;
    }

#endif

    return beam_matrix;
}

/**
 * Computes the broadband scattering strength for a specific interface.
 */
bool envelope_generator::scattering( size_t interface, const wposition1& location,
		const seq_vector& frequencies, double de_incident,
		double de_scattered, double az_incident, double az_scattered,
		vector<double>* amplitude)
{
	switch ( interface ) {
	case eigenverb::BOTTOM:
		_ocean->bottom().scattering(location,frequencies,
				de_incident,de_scattered,az_incident,az_scattered,
				amplitude) ;
		break;
	case eigenverb::SURFACE:
		_ocean->surface().scattering(location,frequencies,
				de_incident,de_scattered,az_incident,az_scattered,
				amplitude) ;
		break;
	default:
		size_t layer = (size_t) floor( (interface-2.0)/2.0 ) ;
		_ocean->volume( layer ).scattering(location,frequencies,
				de_incident,de_scattered,az_incident,az_scattered,
				amplitude) ;
		break;
	}

	// TODO Check for threshold
//	BOOST_FOREACH (double amp, *amplitude) {
//	    if (amp <= intensity_threshold ) {
//	        return true;
//	    }
//	}
	// Threshold not met
	return true;
}
