/**
 * @example eigenverb/test/eigenverb_test.cc
 */

#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>
#include <boost/progress.hpp>
#include <usml/waveq3d/waveq3d.h>
#include <usml/eigenverb/eigenverb_collection.h>
#include <usml/eigenverb/envelope_collection.h>
#include <usml/eigenverb/eigenverb_interpolator.h>
#include <boost/numeric/ublas/vector_proxy.hpp>
#include <iostream>     // std::cout, std::fixed
#include <iomanip>      // std::setprecision

BOOST_AUTO_TEST_SUITE(eigenverb_test)

using namespace boost::unit_test;
using namespace usml::waveq3d;

/**
 * @ingroup eigenverb_test
 * @{
 */

static const double time_step = 0.100 ;
static const double src_lat = 45.0;        // location = mid-Atlantic
static const double src_lng = -45.0;
static const double c0 = 1500.0;           // constant sound speed

/**
 * Tests the basic features of the eigenverb generation process.
 *
 *   - Profile: constant 1500 m/s sound speed, Thorp absorption
 *   - Bottom: 1000 meters, sand
 *   - Source: 45N, 45W, on surface, 1000 Hz
 *   - Interfaces: bottom, surface, and volume
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
 *		- P = path length.
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
 * The length and width of the eigenverb are computed using
 * <pre>
 * 		length = L * dDe / sin(grazing)
 * 		width = L * dAZ * cos(DE)
 * 	</pre>
 * where
 * 		- dDE = initial spacing between rays in DE direction (radians)
 * 		- dAZ = initial spacing between rays in AZ direction (radians)
 *
 * @xref Sean Reilly, David Thibaudeau, Ted Burns,
 * 		 "Fast computation of reverberation using Gaussian beam reflections",
 * 		 report prepared for NAWCTSD
 * @xref Sean Reilly, Gopu Potty, "Verification Tests for Hybrid Gaussian
 *		 Beams in Spherical/Time Coordinates", 10 May 2012.
 */
BOOST_AUTO_TEST_CASE( eigenverb_basic ) {
    cout << "=== eigenverb_test: eigenverb_basic ===" << endl;
    const char* ncname = USML_TEST_DIR "/eigenverb/test/eigenverb_basic_";
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

    seq_log freq( 1000.0, 10.0, 1 );
    wposition1 pos( src_lat, src_lng, 0.0 );
    seq_linear de( -80.0, de_spacing, 60.0 );
    seq_linear az( -40.0, az_spacing, 40.1 );

    // build a wavefront that just generates eigenverbs

    eigenverb_collection* eigenverbs = new eigenverb_collection( ocean.num_volume() ) ;
    wave_queue wave( ocean, freq, pos, de, az, time_step ) ;
    wave.add_eigenverb_listener(eigenverbs) ;

    while ( wave.time() < time_max ) {
        wave.step();
    }

    // record eigenverbs for each interface to their own disk file

    for ( int n=0 ; n < eigenverbs->num_interfaces() ; ++n ) {
    	std::ostringstream filename ;
    	filename << ncname << n << ".nc" ;
    	eigenverbs->write_netcdf( filename.str().c_str(),n) ;
    }

    // test the accuracy of the eigenverb contributions
    // just tests downward facing rays to the bottom, along az=0

    const eigenverb_list& list = eigenverbs->eigenverbs(eigenverb::BOTTOM) ;
    BOOST_FOREACH( eigenverb verb, list ) {
		if ( verb.source_de < 0.0 && verb.source_az == 0.0 ) {

			// compute path length to first bottom bounce on a spherical earth

			int segments = verb.bottom + verb.surface + 1;
			double R = wposition::earth_radius ;
			double Rb = wposition::earth_radius - depth ;
			double p = R * sin(abs(verb.source_de)) ;
			double q = R * R - Rb * Rb ;
			double path_length = p - sqrt( p*p - q ) ; // quadratic equation

			// compute gazing angle, complete path length, and time of arrival

			double alpha = acos( ( R * R + Rb * Rb - path_length * path_length )
					/ ( 2 * R * Rb ) ) ;
			double grazing = abs(verb.source_de) - alpha ;
			path_length *= segments ;
			double time = path_length / c0 ;

			// compute height, width, and area area centered on ray

			const double de_angle = verb.source_de ;
			const double de_plus  = de_angle + 0.5 * to_radians(de_spacing) ;
			const double de_minus = de_angle - 0.5 * to_radians(de_spacing) ;

			const double az_angle = verb.source_az ;
			const double az_plus  = az_angle + 0.5 * to_radians(az_spacing) ;
			const double az_minus = az_angle - 0.5 * to_radians(az_spacing) ;

			const double area = (sin(de_plus) - sin(de_minus)) * (az_plus - az_minus);
			const double de_delta = de_plus - de_minus ;	// average height
			const double az_delta = area / de_delta ;		// average width

			double verb_length = path_length * de_delta / sin(grazing) ;
			double verb_width = path_length * az_delta ;

			// compare to results computed by model
			// 	  - accuracy of length/width just based on prior measurements,
			//    - length/width errors as high as 0.1 meters would still be good

			cout << std::fixed << std::setprecision(4)
				 << "de=" << to_degrees(verb.source_de)
				 << " s=" << verb.surface
				 << " b=" << verb.bottom
				 << "\tL=" << sqrt(verb.length2)
				 << " theory=" << verb_length
				 << "\tW=" << sqrt(verb.width2)
				 << " theory=" << verb_width
				 << endl ;
			BOOST_CHECK_SMALL( verb.time-time, 1e-3 ) ;
			BOOST_CHECK_SMALL( verb.grazing-grazing, 1e-6 ) ;
			BOOST_CHECK_SMALL( verb.direction-verb.source_az, 1e-6 ) ;
			BOOST_CHECK_SMALL( sqrt(verb.length2) - verb_length, 0.005 ) ;
			BOOST_CHECK_SMALL( sqrt(verb.width2) - verb_width, 0.005 ) ;
		}
    }

    // clean up and exit

    delete eigenverbs ;
}

/**
 * Tests the the eigenverb generation process using conditions like
 * those used in the eigenverb_demo.m scenario.
 *
 *   - Profile: constant 1500 m/s sound speed, no absorption
 *   - Bottom: 200 meters, sand
 *   - Source: 45N, 45W, on surface, 1000 Hz
 *   - Interfaces: bottom and surface, but limit to 2 bounces
 *   - Time Step: 100 msec
 *   - Launch D/E: 91 tangent spaced rays from -90 to +90 degrees
 *   - Launch AZ: Rays in the range [0,360) with 20 degree spacing.
 *
 * The primary motivation for this test is to generate an eigenverb netCDF
 * file that can be used to support off-line comparisons to the
 * eigenverb_demo.m scenario. The secondary motivation is to test un-even
 * ray spacing and test limiting the outputs to direct path. In addition to
 * these goals, it also automatically checks the accuracy of the eigenverbs
 * for the bottom to the analytic solution in the reverberation paper.
 */
BOOST_AUTO_TEST_CASE( eigenverb_analytic ) {
    cout << "=== eigenverb_test: eigenverb_analytic ===" << endl;
    const char* ncname = USML_TEST_DIR "/eigenverb/test/eigenverb_analytic_";
    const char* ncname_wave = USML_TEST_DIR "/eigenverb/test/eigenverb_analytic_wave.nc";
    const double time_max = 4.0;
    const double depth = 200.0 ;

    // initialize propagation model

    attenuation_model* attn = new attenuation_constant(0.0);
	profile_model* profile = new profile_linear(c0, attn);
	boundary_model* surface = new boundary_flat();
	reflect_loss_model* bottom_loss = new reflect_loss_rayleigh(reflect_loss_rayleigh::SAND) ;
	boundary_model* bottom = new boundary_flat(depth,bottom_loss);
	ocean_model ocean(surface, bottom, profile);

    seq_log freq( 1000.0, 10.0, 1 );
    wposition1 pos( src_lat, src_lng, 0.0 );
    seq_rayfan de( -90.0, 90.0, 181 );
    seq_linear az( 0.0, 20.0, 359.0 );

    // build a wavefront that just generates eigenverbs

    eigenverb_collection* eigenverbs = new eigenverb_collection( ocean.num_volume() ) ;
    wave_queue wave( ocean, freq, pos, de, az, time_step ) ;
    wave.add_eigenverb_listener(eigenverbs) ;
    wave.max_bottom(2) ;
    wave.max_surface(2) ;

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

    // test the accuracy of the eigenverb contributions
    // just tests downward facing rays to the bottom, along az=0
    // because those are the rays which we have analytic solutions for

    const eigenverb_list& list = eigenverbs->eigenverbs(eigenverb::BOTTOM) ;
    BOOST_FOREACH( eigenverb verb, list ) {
		if ( verb.source_de < 0.0 && verb.source_az == 0.0
			&& verb.surface == 0 && verb.bottom == 0 )
		{

			// compute path length to first bottom bounce on a spherical earth

			int segments = verb.bottom + verb.surface + 1;
			double R = wposition::earth_radius ;
			double Rb = wposition::earth_radius - depth ;
			double p = R * sin(abs(verb.source_de)) ;
			double q = R * R - Rb * Rb ;
			double path_length = p - sqrt( p*p - q ) ; // quadratic equation

			// compute gazing angle, complete path length, and time of arrival

			double alpha = acos( ( R * R + Rb * Rb - path_length * path_length )
					/ ( 2 * R * Rb ) ) ;
			double grazing = abs(verb.source_de) - alpha ;
			path_length *= segments ;
			double time = path_length / c0 ;

			// compute height, width, and area area centered on ray

			const double de_angle = verb.source_de ;
			const double de_plus  = de_angle
				+ 0.5 * to_radians( de.increment(verb.de_index) ) ;
			const double de_minus = de_angle
				- 0.5 * to_radians( de.increment(verb.de_index-1) ) ;

			const double az_angle = verb.source_az ;
			const double az_plus  = az_angle
				+ 0.5 * to_radians( az.increment(verb.az_index) ) ;
			const double az_minus = az_angle
				- 0.5 * to_radians( az.increment(verb.az_index-1) ) ;

			const double area = (sin(de_plus) - sin(de_minus)) * (az_plus - az_minus);
			const double de_delta = de_plus - de_minus ;	// average height
			const double az_delta = area / de_delta ;		// average width

			double verb_length = path_length * de_delta / sin(grazing) ;
			double verb_width = path_length * az_delta ;

			// compare to results computed by model
			// 	  - accuracy of length/width just based on prior measurements,
			//    - length/width errors as high as 0.1 meters would still be good

			cout << std::fixed << std::setprecision(4)
				 << "de=" << to_degrees(verb.source_de)
				 << " s=" << verb.surface
				 << " b=" << verb.bottom
				 << "\tL=" << sqrt(verb.length2)
				 << " theory=" << verb_length
				 << "\tW=" << sqrt(verb.width2)
				 << " theory=" << verb_width
				 << endl ;
			BOOST_CHECK_SMALL( verb.time-time, 1e-3 ) ;
			BOOST_CHECK_SMALL( verb.grazing-grazing, 1e-6 ) ;
			BOOST_CHECK_SMALL( verb.direction-verb.source_az, 1e-6 ) ;
			BOOST_CHECK_SMALL( sqrt(verb.length2)-verb_length, 0.05 ) ;
			BOOST_CHECK_SMALL( sqrt(verb.width2)-verb_width, 0.05 ) ;
		}
    }

    // clean up and exit

    delete eigenverbs ;
}

/**
 * Test the ability to generate a individual envelope contributions and
 * write envelopes out to netCDF.  The eigenverbs are filled in "by hand"
 * instead of being calculated from physical principles. This gives us
 * better isolation between the testing of the eigenverb and envelope models.
 *
 *   - Profile: constant 1500 m/s sound speed
 *   - Frequencies: 1000, 2000, 3000 Hz
 *   - Scattering strength = 0.10, 0.11, 0.12 (linear units)
 *   - Pulse length = 1.0 sec
 *   - Grazing angle = 30 deg
 *   - Depth 1000 meters.
 *   - Eigenverb Power = 0.2. 0.2, 0.2 (linear units)
 *   - Eigenverb length = 20.0 meters
 *   - Eigenverb width = 10.0 meters
 *
 * One envelope contribution is created at a round trip travel time of
 * 10 seconds.  A second contribution, with have the power, is created
 * at 30 seconds. This tests the ability to accumulate an envelope from
 * multiple contributions.
 *
 * Automatically compares the peaks of the first contribution to the
 * monostatic solution.
 * $f[
 *		I_{monostatic} = \frac{ 0.5 T_0 E_s^2 \sigma }
 *							  { Tsr \sqrt{ 4 L_s^2 W_s^2 } }
 * $f]
 * Writes reverberation envelopes to the envelope_basic.nc files.
 */
BOOST_AUTO_TEST_CASE( envelope_basic ) {
    cout << "=== eigenverb_test: envelope_basic ===" << endl;
    const char* ncname = USML_TEST_DIR "/eigenverb/test/envelope_basic.nc";

    // setup scenario for 30 deg D/E in 1000 meters of water

    double angle = M_PI / 6.0 ;
    double depth = 1000.0 ;
    double range = sqrt(3.0) * depth / ( 1852.0 * 60.0 );
    double power = 0.2 ;
    double pulse_length = 1.0 ;

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
	verb.power = vector<double>( freq.size(), power ) ;
	verb.length = 20.0 ;
	verb.width = 10.0 ;
	verb.length2 = verb.length * verb.length ;
	verb.width2 = verb.width * verb.width ;

	// construct an envelope_collection

	const seq_vector* travel_time = new seq_linear(0.0,0.1,400.0) ;
	envelope_collection envelopes(
		&freq,			// envelope_freq
		0,				// src_freq_first
		travel_time,	// travel_time, cloned by model
		40.0,			// reverb_duration
		pulse_length,	// pulse_length
		1e-30,			// threshold
		1, 				// num_azimuths
		1, 				// num_src_beams
		1,   			// num_rcv_beams
		0.0,            // initial_time - fill the api
		1,              // source_id - fill the api
		1,              // receiver_id - fill the api
		wposition1(0.0,0.0),   // src_pos - fill the api
	    wposition1(0.0,0.0) ); // rcv_pos - fill the api

    delete travel_time;

	vector<double> scatter( freq.size() ) ;
	matrix<double> src_beam( freq.size(), 1, 1.0 ) ;
	matrix<double> rcv_beam( freq.size(), 1, 1.0 ) ;
	for ( size_t f=0 ; f < freq.size() ; ++f ) {
		scatter[f] = 0.1 + 0.01 * f ;
	}

	// add contributions at t=10 and t=30 sec

	verb.time = 5.0 ;
	envelopes.add_contribution( verb, verb,
		src_beam, rcv_beam, scatter, 0.0, 0.0 ) ;

	verb.time = 15.0 ;
	verb.power *= 0.5 ;
	envelopes.add_contribution( verb, verb,
			src_beam, rcv_beam, scatter, 0.0, 0.0 ) ;

	envelopes.write_netcdf(ncname) ;

	// compare intensity to analytic solution for monostatic result (eqn. 31).
	// - divides total energy by duration to estimate peak
	// - note that (Ls2+Lr2)*(Ws2+Wr2) = 2 Ls2 Ws2 when s=r
	// - includes extra 4 pi in denominator, based on Matlab results
	// TODO find source of mystery scaling factors (0.25 * 0.5)

	double factor = cos(angle) / c0 ;
	double sigma2 = verb.length2 / 2.0 ;
	double duration = 0.5 * sqrt( pulse_length*pulse_length + factor*factor*sigma2 ) ;
	vector<double> theory = 10.0*log10(
			0.25 * 0.5 * pulse_length * power * power * scatter
			/ sqrt(4.0 * verb.length2 * verb.width2) / duration ) ;
	size_t index = 105 ;
	cout << "duration=" << duration << endl;
	for (size_t f = 0; f < freq.size(); ++f) {
		double model = 10.0*log10(envelopes.envelope(0,0,0)(f,index));
		cout << "theory=" << theory[f] << " model=" << model << endl;
		BOOST_CHECK_SMALL( abs(model-theory[f]), 1e-4);
	}
}

/**
 * Test the ability to compute source and receiver eigenverbs at different
 * frequencies.  Similar to envelope_basic test except that:
 *
 * 		- source and receiver are at different frequencies
 * 		- receiver is interpolated onto envelope frequency axis
 * 		- result is limited to first two source frequencies
 */
BOOST_AUTO_TEST_CASE( envelope_interpolate ) {
    cout << "=== eigenverb_test: envelope_interpolate ===" << endl;
    const char* ncname = USML_TEST_DIR "/eigenverb/test/envelope_interpolate.nc";

    // setup scenario for 30 deg D/E in 1000 meters of water

    double angle = M_PI / 6.0 ;
    double depth = 1000.0 ;
    double range = sqrt(3.0) * depth / ( 1852.0 * 60.0 );
    double power = 0.2 ;
    double pulse_length = 1.0 ;

	// build a simple source eigenverb

	eigenverb src_verb ;
	src_verb.time = 0.0 ;
	src_verb.position = wposition1(range,0.0,-depth) ;
	src_verb.direction = 0.0 ;
	src_verb.grazing = angle ;
	src_verb.sound_speed = c0 ;
	src_verb.de_index = 0 ;
	src_verb.az_index = 0 ;
	src_verb.source_de = -angle ;
	src_verb.source_az = 0.0 ;
	src_verb.surface = 0 ;
	src_verb.bottom = 0 ;
	src_verb.caustic = 0 ;
	src_verb.upper = 0 ;
	src_verb.lower = 0 ;

	seq_linear src_freq(1000.0,1000.0,3) ;
	src_verb.frequencies = &src_freq ;
	src_verb.power = vector<double>( src_freq.size(), power ) ;
	src_verb.length = 20.0 ;
	src_verb.width = 10.0 ;
	src_verb.length2 = src_verb.length * src_verb.length ;
	src_verb.width2 = src_verb.width * src_verb.width ;

	// build a simple receiver eigenverb
	// identical to src_verb except for frequency axis

	eigenverb rcv_verb_original ;
	rcv_verb_original.time = 0.0 ;
	rcv_verb_original.position = wposition1(range,0.0,-depth) ;
	rcv_verb_original.direction = 0.0 ;
	rcv_verb_original.grazing = angle ;
	rcv_verb_original.sound_speed = c0 ;
	rcv_verb_original.de_index = 0 ;
	rcv_verb_original.az_index = 0 ;
	rcv_verb_original.source_de = -angle ;
	rcv_verb_original.source_az = 0.0 ;
	rcv_verb_original.surface = 0 ;
	rcv_verb_original.bottom = 0 ;
	rcv_verb_original.caustic = 0 ;
	rcv_verb_original.upper = 0 ;
	rcv_verb_original.lower = 0 ;

	seq_linear rcv_freq(500.0,200.0,10) ;
	rcv_verb_original.frequencies = &rcv_freq ;
	rcv_verb_original.power = vector<double>( rcv_freq.size(), power ) ;
	rcv_verb_original.length = 20.0 ;
	rcv_verb_original.width = 10.0 ;
	rcv_verb_original.length2 = rcv_verb_original.length * rcv_verb_original.length ;
	rcv_verb_original.width2 = rcv_verb_original.width * rcv_verb_original.width ;

	// interpolate rcv_verb_original onto frequency axis of envelope

	seq_linear envelope_freq( 1000.0, 1000.0, 2 ) ;
	eigenverb rcv_verb ;
	rcv_verb.frequencies = &envelope_freq ;
	rcv_verb.power = vector<double>( envelope_freq.size() ) ;

	eigenverb_interpolator interpolator( &rcv_freq,  &envelope_freq ) ;
	interpolator.interpolate( rcv_verb_original, &rcv_verb ) ;

	// construct an envelope_collection

	const seq_vector* travel_time = new seq_linear(0.0,0.1,400.0) ;
	envelope_collection envelopes(
		&envelope_freq,			// envelope_freq
		0,				// src_freq_first
		travel_time,	// travel_time, cloned by model
		40.0,			// reverb_duration
		1.0, 			// pulse_length
		1e-30,			// threshold
		1, 				// num_azimuths
		1, 				// num_src_beams
		1,              // num_rcv_beams
		0.0,            // initial_time - fill the api
        1,              // source_id - fill the api
        1,              // receiver_id - fill the api
        wposition1(0.0,0.0),   // src_pos - fill the api
        wposition1(0.0,0.0) ); // rcv_pos - fill the api

    delete travel_time;

	vector<double> scatter( envelope_freq.size() ) ;
	matrix<double> src_beam( envelope_freq.size(), 1, 1.0 ) ;
	matrix<double> rcv_beam( envelope_freq.size(), 1, 1.0 ) ;
	for ( size_t f=0 ; f < envelope_freq.size() ; ++f ) {
		scatter[f] = 0.1 + 0.01 * f ;
	}

	// add contributions at t=10 and t=30 sec

	src_verb.time = 5.0 ;
	rcv_verb.time = 5.0 ;
	envelopes.add_contribution( src_verb, rcv_verb,
		src_beam, rcv_beam, scatter, 0.0, 0.0 ) ;

	src_verb.time = 15.0 ;
	rcv_verb.time = 15.0 ;
	src_verb.power *= 0.5 ;
	rcv_verb.power *= 0.5 ;
	envelopes.add_contribution( src_verb, rcv_verb,
			src_beam, rcv_beam, scatter, 0.0, 0.0 ) ;

	envelopes.write_netcdf(ncname) ;

	// compare intensity to analytic solution for monostatic result (eqn. 31).
	// - divides total energy by duration to estimate peak
	// - note that (Ls2+Lr2)*(Ws2+Wr2) = 2 Ls2 Ws2 when s=r
	// - includes extra 4 pi in denominator, based on Matlab results
	// TODO find source of mystery scaling factors (0.25 * 0.5)

	double factor = cos(angle) / c0 ;
	double sigma2 = src_verb.length2 / 2.0 ;
	double duration = 0.5 * sqrt( pulse_length*pulse_length + factor*factor*sigma2 ) ;
	vector<double> theory = 10.0*log10(
			0.25 * 0.5 * pulse_length * power * power * scatter
			/ sqrt(4.0 * src_verb.length2 * src_verb.width2) / duration ) ;
	size_t index = 105 ;
	cout << "duration=" << duration << endl;
	for (size_t f = 0; f < envelope_freq.size(); ++f) {
		double model = 10.0*log10(envelopes.envelope(0,0,0)(f,index));
		cout << "theory=" << theory[f] << " model=" << model << endl;
		BOOST_CHECK_SMALL( abs(model-theory[f]), 1e-4);
	}
}

/**
 * Test the ability to insert source eigenverbs generated from eigenverb_basic
 * test into a boost rtree and query them with an expected result.
 *      - All four volume interfaces are inserted into one rtree.
 *          Production code uses one rtree per interface.
 *      - This test uses points as the keys as they are faster to create than boxes
 *      - This test bulk inserts a collection_pairs std::list into the rtree
 *          constructor with iterator arguments.
 */

using namespace boost::geometry;

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

typedef bg::model::point<double, 2, bg::cs::cartesian > point;

typedef bg::model::box<point> box;

typedef std::pair<point, eigenverb_list::iterator> value_pair;

typedef bgi::rtree<value_pair, bgi::rstar<16,4> > rtree_type;

BOOST_AUTO_TEST_CASE( rtree_basic ) {

    cout << "=== eigenverb_test: rtree_basic ===" << endl;
    const char* ncname = USML_TEST_DIR "/eigenverb/test/eigenverb_basic_";

    int interfaces = 4;
    eigenverb_collection collection(interfaces);
    eigenverb_list eigenverbs;
    eigenverb verb;

    // Use local pairs to package in rtree
    std::list<value_pair> collection_pairs;
    eigenverb_list::iterator iter;

    int i = 0;
    // Read eigenverbs for each interface from their own disk file
    for ( int n=0 ; n < interfaces ; ++n ) {
        std::stringstream filename ;
        filename << ncname << n << ".nc" ;
        eigenverbs = collection.read_netcdf( filename.str().c_str(),n) ;

        // get eigenverb values
        for (iter = eigenverbs.begin(); iter != eigenverbs.end(); ++iter ) {

            verb = *iter;
            collection_pairs.push_back(
                            std::make_pair(point(verb.position.latitude(),
                            verb.position.longitude()), iter));
            ++i;
        }
    }

    // Use Packed constructor of rtree for fastest insertion
    rtree_type rtree = rtree_type(collection_pairs.begin(), collection_pairs.end());

    // Test query box creation
    double q = 0.0;
    double delta_lat = 0.0;
    double delta_long = 0.0;
    double latitude = 0.0;
    double longitude = 0.0;

    // meters/degree  60 nmiles/degree * 1852 meters/nmiles
    double lat_scaler = (60.0*1852.0);

    // Use receiver eigenverbs lat, long, length and width
    // to create a bounding box.
    double rcv_verb_length2 = 8000.0; // meters squared
    double rcv_verb_width2 = 8000.0; // meters squared
    double rcv_verb_latitude = 45.0;  // North
    double rcv_verb_longitude = -45.0;// East

    q = sqrt(max(rcv_verb_length2, rcv_verb_width2));
    latitude = rcv_verb_latitude;
    longitude = rcv_verb_longitude;
    delta_lat = q/lat_scaler;
    delta_long = q/(lat_scaler * cos(to_radians(latitude)));

    // create a box, first point bottom left, second point upper right
    box query_box(point( latitude - delta_lat, longitude - delta_long),
                point(latitude + delta_lat, longitude + delta_long));

    std::cout << "spatial query box:" << std::endl;
    std::cout << bg::wkt<box>(query_box) << std::endl;

    std::vector<value_pair> result_s;
    rtree.query(bgi::within(query_box), std::back_inserter(result_s));

    // display results
    std::cout << "spatial query result:" << std::endl;
    BOOST_CHECK_EQUAL(result_s.size(), 4);
    if (result_s.size() != 0) {
        BOOST_FOREACH(value_pair const& v, result_s)
                std::cout << bg::wkt<point>(v.first) << std::endl;
        cout << " Found " << result_s.size() << " results from " << i << " eigenverbs" << endl;
    } else {
        cout << " No results found " << endl;
    }

    std::cout << "=== rtree_basic: test completed! ===" << std::endl;

}

/// @}

BOOST_AUTO_TEST_SUITE_END()
