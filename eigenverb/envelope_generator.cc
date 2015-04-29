/**
 * @file envelope_generator.cc
 * Computes reverberation envelopes from eigenverbs.
 */
#include <usml/eigenverb/envelope_generator.h>
#include <usml/eigenverb/eigenverb.h>
#include <boost/foreach.hpp>

using namespace usml::eigenverb ;

/**
 * Default pulse length of the signal (sec).
 */
double envelope_generator::pulse_length = 1.0 ;

/**
 * Default duration of the reverberation envelope (sec).
 */
double envelope_generator::time_maximum = 40.0 ;

/**
 * Default sampling period for the reverberation envelope (sec).
 */
double envelope_generator::time_step = 0.1 ;

/**
 * Minimum intensity level for valid reverberation contributions (dB).
 */
double envelope_generator::threshold = -300.0 ;


/**
 * Copies envelope computation parameters from static memory into
 * this specific task.
 */
envelope_generator::envelope_generator(
		const seq_vector* transmit_freq,
		const seq_vector* receiver_freq,
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
	_eigenverb_interpolator(transmit_freq,receiver_freq),
	_envelopes( new envelope_collection(
		transmit_freq,
		(size_t) ceil( time_maximum / time_step ),
		time_step,
		pulse_length,
		pow(10.0,threshold/10.0),
		num_azimuths,
		num_src_beams,
		num_rcv_beams ) )
{
}

/**
 * Executes the Eigenverb reverberation model.
 */
void envelope_generator::run() {
	BOOST_FOREACH( eigenverb rcv_verb, _rcv_eigenverbs.get() ) {

	}
	this->notify_envelope_listeners(_envelopes) ;
}

/**
 *
 */

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
