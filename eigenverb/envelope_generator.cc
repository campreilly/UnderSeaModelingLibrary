/**
 * @file envelope_generator.cc
 * Computes reverberation envelopes from eigenverbs.
 */
#include <usml/eigenverb/envelope_generator.h>
#include <usml/eigenverb/eigenverb.h>
#include <usml/threads/smart_ptr.h>
#include <boost/foreach.hpp>

using namespace usml::eigenverb ;

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
	_ocean( ocean_shared::current() ),
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
		num_rcv_beams ) ) ;
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

				scattering( interface,
					rcv_verb.position, *freq,
					src_verb.grazing, rcv_verb.grazing,
					src_verb.direction, rcv_verb.direction,
					&scatter ) ;

				// compute beam levels
				//
				// TODO Replace the scalar_matrix versions of src_beam and rcv_beam
				// 		with real beam pattern results.

				src_beam = scalar_matrix<double>(num_freq,num_src_beams,1.0);
				rcv_beam = scalar_matrix<double>(num_freq,num_rcv_beams,1.0);

				// create envelope contribution

				_envelopes->add_contribution( src_verb, rcv_verb,
						src_beam, rcv_beam, scatter, xs2, ys2 ) ;
			}
		}
	}
	this->notify_envelope_listeners(_envelopes) ;
}

/**
 * Computes the broadband scattering strength for a specific interface.
 */
void envelope_generator::scattering( size_t interface, const wposition1& location,
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
}
