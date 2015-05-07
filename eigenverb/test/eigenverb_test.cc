/**
 * @example eigenverb/test/eigenverb_test.cc
 */

#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>
#include <usml/waveq3d/waveq3d.h>
#include <usml/eigenverb/eigenverb_collection.h>
#include <usml/eigenverb/envelope_collection.h>

BOOST_AUTO_TEST_SUITE(eigenverb_test)

using namespace boost::unit_test;
using namespace usml::waveq3d;

/**
 * @ingroup waveq3d_test
 * @{
 */

static const double time_step = 0.100 ;
static const double src_lat = 45.0;        // location = mid-Atlantic
static const double src_lng = -45.0;
static const double c0 = 1500.0;           // constant sound speed

/**
 * Tests the basic features of the eigenverb generation process.
 *
 * - Scenario parameters
 *   - Profile: constant 1500 m/s sound speed, Thorp absorption
 *   - Bottom: 1000 meters, sand
 *   - Source: 45N, 45W, on surface, 100, 1000, and 10000 Hz
 *   - Target: 45.02N, 45W, on bottom
 *   - Time Step: 100 msec
 *   - Launch D/E: 5 degree linear spacing from -60 to 60 degrees
 *   - Launch AZ: 10 degree linear spacing from -40 to 40 degrees
 *
 * Automatically checks the accuracy of the eigenverbs for the bottom
 * to the analytic solution in the reverberation paper.
 *
 * To maximize accuracy we compute path length and angles on a round earth
 * with a flat bottom, using eqn. (25) - (27) from the verification test report.
 * For a path with a given DE (where negative is down), the path length for
 * the first interaction with the bottom is found by solving eqn. (25) for L:
 * <pre>
 *     Rb^2 = R^2 + L^2 - 2 R L sin(DE)
 *     L^2 - 2 R L sin(DE) + (R^2 - Rb^2) = 0
 * </pre>
 * where
 * 		- R = source distance from earth center,
 *		- Rb = bottom distance from earth center,
 *		- DE = launch D/E angle, and
 *		- L = path length.
 *
 * The quadratic equation solution for the path length is
 * <pre>
 * 		p = R sin(abs(DE))
 * 		q = R^2 - Rb^2
 * 		L = p - sqrt( p*p - q )
 * </pre>
 * The negative root has been chosen to make an acute angle between Rs and Rb.
 * The angle between Rs and Rb is given by:
 * <pre>
 * 		L^2 = R^2 + Rb^2 - 2 R Rb cos(alpha) ;
 * 		alpha = acos[ ( Rs^2 + Rb^2 - L^2 ) / (2 Rs Rb) ]
 * </pre>
 * The time of arrival and grazing angle are given by:
 * <pre>
 * 		time = L / c ;
 * 		grazing = DE - alpha
 * </pre>
 * @xref Sean Reilly, David Thibaudeau, Ted Burns,
 * 		 "Fast computation of reverberation using Gaussian beam reflections",
 * 		 report prepared for NAWCTSD
 * @xref Sean Reilly, Gopu Potty, "Verification Tests for Hybrid Gaussian
 *		 Beams in Spherical/Time Coordinates", 10 May 2012.
 */
BOOST_AUTO_TEST_CASE( eigenverb_basic ) {
    cout << "=== eigenverb_test: eigenverb_basic ===" << endl;
    const char* ncname = USML_TEST_DIR "/eigenverb/test/eigenverb_basic_";
    const char* ncname_wave = USML_TEST_DIR "/eigenverb/test/eigenverb_basic_wave.nc";
    const double time_max = 3.5;
    const double depth = 1000.0 ;
    const double de_spacing = 5.0 ;
    const double az_spacing = 10.0 ;

    // initialize propagation model

	profile_model* profile = new profile_linear(c0);
	boundary_model* surface = new boundary_flat();
	reflect_loss_model* bottom_loss = new reflect_loss_rayleigh(reflect_loss_rayleigh::SAND) ;
	boundary_model* bottom = new boundary_flat(depth,bottom_loss);
	ocean_model ocean(surface, bottom, profile);
	volume_model* layer = new volume_flat(300.0, 10.0, -40.0);
	ocean.add_volume(layer);

    seq_log freq( 100.0, 10.0, 10e3 );
    wposition1 pos( src_lat, src_lng, 0.0 );
    seq_linear de( -60.0, de_spacing, 60.0 );
    seq_linear az( -40.0, az_spacing, 40.1 );

    // build a wavefront that just generates eigenverbs

    eigenverb_collection* eigenverbs = new eigenverb_collection( ocean.num_volume() ) ;
    wave_queue wave( ocean, freq, pos, de, az, time_step ) ;
    wave.add_eigenverb_listener(eigenverbs) ;

    // propagate rays and record wavefronts to disk.

    cout << "propagate wavefronts for " << time_max << " seconds" << endl;
    cout << "writing wavefronts to " << ncname_wave << endl;

    wave.init_netcdf( ncname_wave );
    wave.save_netcdf();
    while ( wave.time() < time_max ) {
        wave.step();
        wave.save_netcdf();
    }
    wave.close_netcdf();

    // record eigenverbs for each interface to their own disk file

    for ( int n=0 ; n < eigenverbs->num_interfaces() ; ++n ) {
    	std::ostringstream filename ;
    	filename << ncname << n << ".nc" ;
    	eigenverbs->write_netcdf( filename.str().c_str(),n) ;
    }

    // compute the sound speed at the bottom for rofile->flat_earth(true) ;
//    const double c_bottom = c0 * (wposition::earth_radius-depth)
//    		/ wposition::earth_radius ;

    // test the accuracy of the eigenverb contributions
    // just tests downward facing rays to the bottom, along az=0

    const eigenverb_list& list = eigenverbs->eigenverbs(eigenverb::BOTTOM) ;
    BOOST_FOREACH( eigenverb verb, list ) {
		if ( verb.source_de < 0.0 && verb.source_az == 0.0
			&& verb.surface == 0 && verb.bottom == 0 )
		{
			int segments = verb.bottom + verb.surface + 1;
			double R = wposition::earth_radius ;
			double Rb = wposition::earth_radius - depth ;
			double p = R * sin(abs(verb.source_de)) ;
			double q = R * R - Rb * Rb ;
			double path_length = p - sqrt( p*p - q ) ; // quadratic equation

			double alpha = acos( ( R * R + Rb * Rb - path_length * path_length )
					/ ( 2 * R * Rb ) ) ;
			double grazing = abs(verb.source_de) - alpha ;
			path_length *= segments ;
			double time = path_length / c0 ;

			double L = 2.0 * path_length * to_radians(de_spacing) ;
			double W = 2.0 * path_length * to_radians(az_spacing) * cos(verb.source_de);
			vector<double> spread = abs2( TWO_PI * c0 / (*verb.frequencies) ) ;
			vector<double> verb_length = sqrt( spread + L*L ) / sin(grazing) ;
			vector<double> verb_width = sqrt( spread + W*W ) ;

			// compare to results computed by model

			BOOST_CHECK_SMALL( verb.time-time, 1e-3 ) ;
			BOOST_CHECK_SMALL( verb.grazing-grazing, 1e-6 ) ;
			BOOST_CHECK_SMALL( verb.direction-verb.source_az, 1e-6 ) ;
			for ( int f=0; f < verb.frequencies->size() ; ++f ) {
				cout << "f=" << (*verb.frequencies)(f)
							 << "\tL=" << sqrt(verb.length2(f))
							 << " true=" << verb_length(f)
							 << "\tW=" << sqrt(verb.width2(f))
							 << " true=" << verb_width(f)
					 << endl ;
				BOOST_CHECK_SMALL( sqrt(verb.length2(f))-verb_length(f), 0.1 ) ;
				BOOST_CHECK_SMALL( sqrt(verb.width2(f))-verb_width(f), 0.1 ) ;
			}
		}
    }

    // clean up and exit

    delete eigenverbs ;
}

/**
 * Test the ability to generate a single envelope contributions and
 * write it out to netCDF.  Automatically compares the peaks of the
 * first contribution to the monostatic solution.
 * $f[
 *		E_{monostatic} = \pi * E_s^2 * \sigma / \sqrt{ L_s^2 W_s^2 }
 * $f]
 */
BOOST_AUTO_TEST_CASE( envelope_basic ) {
    cout << "=== eigenverb_test: envelope_basic ===" << endl;
    const char* ncname = USML_TEST_DIR "/eigenverb/test/envelope_basic.nc";

    // setup scenario for 30 deg D/E in 1000 meters of water

    double angle = M_PI / 6.0 ;
    double depth = 1000.0 ;
    double range = sqrt(3) * depth / ( 1852.0 * 60.0 );

	// build a simple eigenverb

	eigenverb verb ;
	verb.time = 0.0 ;
	verb.position = wposition1(range,0.0,-depth) ;
	verb.direction = 0.0 ;
	verb.grazing = angle ;
	verb.sound_speed = c0 ;
	verb.de_index = 0 ;
	verb.az_index = 0 ;
	verb.source_de = -angle ;
	verb.source_az = 0.0 ;
	verb.surface = 0 ;
	verb.bottom = 0 ;
	verb.caustic = 0 ;
	verb.upper = 0 ;
	verb.lower = 0 ;

	seq_linear freq(1000.0,1000.0,3) ;
	verb.frequencies = &freq ;
	verb.energy = vector<double>( freq.size() ) ;
	verb.length2 = vector<double>( freq.size() ) ;
	verb.width2 = vector<double>( freq.size() ) ;
	for ( size_t f=0 ; f < freq.size() ; ++f ) {
		verb.energy[f] = 0.2 ;
		verb.length2[f] = 400.0 + 10.0 * f ;
		verb.width2[f] = 100.0 + 10.0 * f ;
	}

	// construct an envelope_collection

	const seq_vector* travel_time = new seq_linear(0.0,0.1,400.0) ;
	envelope_collection collection(
		&freq,			// transmit_freq
		travel_time,	// travel_time
		40.0,			// reverb_duration
		1.0, 			// pulse_length
		1e-30,			// threshold
		1, 				// num_azimuths
		1, 				// num_src_beams
		1 ) ; 			// num_rcv_beams
	delete travel_time ;

	vector<double> scatter( freq.size() ) ;
	matrix<double> src_beam( freq.size(), 1 ) ;
	matrix<double> rcv_beam( freq.size(), 1 ) ;
	for ( size_t f=0 ; f < freq.size() ; ++f ) {
		scatter[f] = 0.1 + 0.01 * f ;
		src_beam(f,0) = 1.0 ;
		rcv_beam(f,0) = 1.0 ;
	}

	// add contributions at t=10 and t=30 sec

	verb.time = 5.0 ;
	collection.add_contribution( verb, verb,
		src_beam, rcv_beam, scatter, 0.0, 0.0 ) ;

	verb.time = 15.0 ;
	verb.energy *= 0.5 ;
	collection.add_contribution( verb, verb,
			src_beam, rcv_beam, scatter, 0.0, 0.0 ) ;

	collection.write_netcdf(ncname) ;

	// compare total energy to analytic solution for monostatic result (eqn. 31).

	const double coeff = 4.0 * M_PI * sqrt(TWO_PI) ;
	vector<double> theory = 10.0*log10(element_div(
		M_PI * 0.2 * 0.2 * scatter,
		sqrt(verb.length2 * verb.width2)))
		- 10.0*log10(coeff * 0.5);
	size_t index = 105 ;
	for (size_t f = 0; f < freq.size(); ++f) {
		double model = 10.0*log10(collection.envelope(0,0,0)(f,index));
		cout << "theory=" << theory[f] << " model=" << model << endl;
		BOOST_CHECK_CLOSE(theory[f], model, 1e-2);
	}
}

/// @}

BOOST_AUTO_TEST_SUITE_END()
