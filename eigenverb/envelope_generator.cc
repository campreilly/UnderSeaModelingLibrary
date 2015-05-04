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

	// create memory for work products

    const seq_vector* freq = _envelopes->transmit_freq() ;
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

				_envelopes->add_contribution( scatter, src_beam, rcv_beam,
						src_verb, rcv_verb ) ;
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
