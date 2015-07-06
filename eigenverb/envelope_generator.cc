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
 * Maximum distance between the peaks of the source and receiver eigenverbs.
 */
double envelope_generator::distance_threshold = 6.0 ;

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
 * Executes the Eigenverb reverberation model.
 */
void envelope_generator::run() {

	// create memory for work products

    const seq_vector* freq = _envelopes->envelope_freq() ;
    const size_t num_freq = freq->size() ;

	vector<double> scatter( num_freq, 1.0 ) ;
	matrix<double> src_beam( num_freq, _envelopes->num_src_beams(), 1.0 ) ;
	matrix<double> rcv_beam( num_freq, _envelopes->num_rcv_beams(), 1.0 ) ;

	eigenverb rcv_verb ;
	rcv_verb.frequencies = freq ;
	rcv_verb.power = vector<double>( num_freq ) ;

	// loop through eigenrays for each interface

	for ( size_t interface=0 ; interface < _rcv_eigenverbs->num_interfaces() ; ++interface) {
//	for ( size_t interface=0 ; interface < 1 ; ++interface) {

		BOOST_FOREACH( eigenverb verb, _rcv_eigenverbs->eigenverbs(interface) ) {
			_eigenverb_interpolator.interpolate(verb,&rcv_verb) ;

			// Cull eigenverbs down with rtree.query
			std::vector<value_pair> result_s;
			_src_eigenverbs->query_rtree(interface, rcv_verb, result_s);

			BOOST_FOREACH( value_pair const& vp, result_s ) {

				eigenverb src_verb = *(vp.second);

				// determine relative range and bearing between the projected Gaussians
				// skip this combo if source peak too far away

			    double bearing ;
			    const double range = rcv_verb.position.gc_range( src_verb.position, &bearing ) ;
			    if ( range > distance_threshold * max(rcv_verb.length,rcv_verb.width)) continue ;

			    if ( range < 1e-6 ) bearing = 0 ;	// fixes bearing = NaN
			    bearing -= rcv_verb.direction ;		// relative bearing

			    const double ys = range * cos( bearing ) ;
			    const double ys2 = ys * ys ;
			    if ( abs(ys) > distance_threshold * rcv_verb.length ) continue ;

			    const double xs = range * sin( bearing ) ;
			    const double xs2 = xs * xs ;
			    if ( abs(xs) > distance_threshold * rcv_verb.width ) continue ;

				// compute interface scattering strength
			    // continue if scattering strength does not meet threshold

				scattering( interface,
					 rcv_verb.position, *freq,
					 src_verb.grazing, rcv_verb.grazing,
					 src_verb.direction, rcv_verb.direction,
					 &scatter )  ;

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
	this->notify_envelope_listeners(_envelopes) ;
}

/**
 * Computes the beam_gain
 */
matrix<double> envelope_generator::beam_gain(
    sensor_params::beam_pattern_list beam_list,
    const seq_vector* freq, double de_rad, double az_rad, orientation orient)
{
    const vector<double> frequencies = *freq;	// beam_level requires ublas vector
    matrix<double> beam_matrix( freq->size(), beam_list.size() ) ;
    vector<double> level( freq->size(), 0.0 ) ;
    BOOST_FOREACH( beam_pattern_model::id_type id, beam_list) {
        beam_pattern_model::reference bp = beam_pattern_map::instance()->find(id);
        bp->beam_level(de_rad, az_rad, orient, frequencies, &level ) ;

        std::copy(level.begin(), level.end(), beam_matrix.begin1());
    }
    return beam_matrix;
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
