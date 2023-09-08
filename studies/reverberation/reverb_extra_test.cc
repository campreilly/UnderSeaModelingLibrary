/**
 * @example studies/reverberation/reverb_extra_test.cc
 *
 * Perform reverb tests that run too slow to be included
 * in the normal suite of regression tests.
 */
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <usml/ocean/ocean_shared.h>
#include <usml/sensors/beams.h>
#include <usml/sensors/sensors.h>
#include <usml/eigenverb/wavefront_generator.h>
#include <list>
#include <boost/progress.hpp>

BOOST_AUTO_TEST_SUITE(reverb_extra_test)

using namespace boost::unit_test;
using namespace usml::waveq3d;

static const double time_step = 0.100 ;
static const double src_lat = 45.0;        // location = mid-Atlantic
static const double src_lng = -45.0;
static const double c0 = 1500.0;           // constant sound speed

/**
 * Tests the the envelope generation process using conditions like
 * those used in the eigenverb_demo.m scenario.
 *
 *   - Profile: constant 1500 m/s sound speed, no absorption
 *   - Surface: Perfectly reflecting, Lambert scattering with -37.0 dB
 *   - Bottom: 200 meters, sand, Lambert scattering with -27.0 dB
 *   - Source: 45N, 45W, on surface, 1000 Hz
 *   - Interfaces: bottom and surface, but limit to 2 bounces
 *   - Time Step: 100 msec
 *   - Launch D/E: 91 tangent spaced rays from -90 to +90 degrees
 *   - Launch AZ: Rays in the range [0,360) with 20 degree spacing.
 *
 * The primary motivation for this test is to generate an envelope netCDF
 * file that can be used to support off-line comparisons to the
 * eigenverb_demo.m scenario.  It also serves as a standalone prototype
 * for the calculations in the rvbenv_generator::run() method.
 *
 * This test artificially limits its processing to downward D/E launch
 * angles and receivers AZ launch angles to zero to match the conditions
 * used in eigenverb_demo.m.
 *
 * It also artificially limits the initial time (using the round trip time to
 * bottom) to test the ability to limit the initial time. Such a limitation is
 * not normally used for a monostatic sensor.
 */
BOOST_AUTO_TEST_CASE( envelope_analytic ) {
    cout << "=== eigenverb_test: envelope_analytic ===" << endl;
    const char* ncname = USML_STUDIES_DIR "/reverberation/envelope_analytic_";
    const char* ncname_env = USML_STUDIES_DIR "/reverberation/envelope_analytic_env.nc";
    const double time_max = 4.0;
    const double depth = 200.0 ;
    const double threshold = 1e-17 ;

    // initialize propagation model

    attenuation_model* attn = new attenuation_constant(0.0);
	profile_model* profile = new profile_linear(c0, attn);

	boundary_model* surface = new boundary_flat();
	surface->scattering( new scattering_lambert(-37.0)  ) ;

	reflect_loss_model* bottom_loss = new reflect_loss_rayleigh(bottom_type_enum::sand) ;
	boundary_model* bottom = new boundary_flat(depth,bottom_loss);
	bottom->scattering( new scattering_lambert()  ) ;

	ocean_model ocean(surface, bottom, profile);

    seq_log freq( 1000.0, 10.0, 1 );
    wposition1 pos( src_lat, src_lng, 0.0 );
    seq_rayfan de( -90.0, 0.0, 91 );
    seq_linear az( 0.0, 20.0, 359.0 );

    // build a wavefront that generates eigenverbs

    eigenverb_collection* eigenverbs = new eigenverb_collection( ocean.num_volume() ) ;
    wave_queue wave( ocean, freq, pos, de, az, time_step ) ;
    wave.add_eigenverb_listener(eigenverbs) ;
    wave.max_bottom(2) ;
    wave.max_surface(2) ;

    // propagate wavefront and compute eigenverbs
    // time the duration of this calculation

    cout << "computing eigenverbs" << endl;
    {
		boost::progress_timer timer ;
		while ( wave.time() < time_max ) {
			wave.step();
		}
    }

    // record eigenverbs for each interface to their own disk file

    for ( int n=0 ; n < eigenverbs->num_interfaces() ; ++n ) {
    	std::ostringstream filename ;
    	filename << ncname << n << ".nc" ;
        cout << "writing eigenverbs to " << filename.str().c_str() << endl;
    	eigenverbs->write_netcdf( filename.str().c_str(),n) ;
    }

	// construct an rvbenv_collection

	seq_vector::csptr  travel_time = new seq_linear(0.0,0.001,70.0) ;
	rvbenv_collection envelopes(
		&freq,			// envelope_freq
		0,				// src_freq_first
		travel_time,	// travel_time, cloned by model
		7.0,			// reverb_duration
		0.25, 			// pulse_length
		threshold,		// threshold
		az.size(), 		// num_azimuths
		1, 				// num_src_beams
		1,              // num_rcv_beams
		2.0*depth/c0,   // initial_time = first dirpath
        1,              // sourceID - fill the api
        1,              // receiver_id - fill the api
        wposition1(src_lat, src_lng),   // src_pos - fill the api
        wposition1(src_lat, src_lng) ); // rcv_pos - fill the api
    delete travel_time;

    // compute the envelope from eigenverbs on each interface

	vector<double> scatter( freq.size() ) ;
	matrix<double> src_beam( freq.size(), 1, 1.0 ) ;
	matrix<double> rcv_beam( freq.size(), 1, 1.0 ) ;

    cout << "computing envelopes" << endl;
    {
//    	int n=0 ;
		boost::progress_timer timer ;
		double distance_threshold = 6.0 ;
		for ( size_t interface_num=0 ; interface_num < 1 ; ++interface_num) {
			for( eigenverb rcv_verb: eigenverbs->eigenverbs(interface_num) ) {
//				cout << "rcv_verb #" << n++ << endl ;
				if ( abs(rcv_verb.source_az) > 1e-6 ) continue ; // just for this test
				for( eigenverb src_verb: eigenverbs->eigenverbs(interface_num) ) {

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

					// compute interface_num scattering strength
					// continue if scattering strength does not meet threshold

					ocean.bottom().scattering(
						rcv_verb.position, freq,
						src_verb.grazing, rcv_verb.grazing,
						src_verb.direction, rcv_verb.direction,
						&scatter )  ;

					// create envelope contribution

					envelopes.add_contribution( src_verb, rcv_verb,
							src_beam, rcv_beam, scatter, xs2, ys2 ) ;
				}
			}
		}
    }
    cout << "writing envelopes to " << ncname_env << endl;
	envelopes.write_netcdf(ncname_env) ;

    // clean up and exit

    delete eigenverbs ;
}

BOOST_AUTO_TEST_SUITE_END()
