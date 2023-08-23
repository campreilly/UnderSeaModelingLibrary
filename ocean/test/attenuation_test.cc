/**
 * @example ocean/test/attenuation_test.cc
 */
#include <boost/test/unit_test.hpp>
#include <usml/ocean/ocean.h>
#include <fstream>

BOOST_AUTO_TEST_SUITE(attenuation_test)

using namespace boost::unit_test;
using namespace usml::ocean;

/**
 * @ingroup ocean_test
 * @{
 */

/**
 * Test the basic features of the attenuation model using
 * the attenuation_constant model.  Given a model where attenuation
 * is 1e-6 * frequency, check to see that the calculated results are
 * within 1e-6% of the analytic values.
 */
BOOST_AUTO_TEST_CASE( constant_attenuation_test ) {
    cout << "=== attenuation_test: constant_attenuation_test ===" << endl;

    // simple values for points and distance

    wposition points(1, 1);
    matrix<double> distance(1, 1);
    distance(0, 0) = 1000.0;

    // compute attenuation

    seq_vector::csptr freq(new seq_log(10.0, 10.0, 7));
    cout << "freq:  " << *freq.get() << endl;
    matrix < vector<double> > atten(1, 1);
    atten(0, 0).resize(freq->size());

    attenuation_constant model(1e-6);
    model.attenuation(points, freq, distance, &atten);
    cout << "atten: " << atten << endl;

    // check the answer

    double value = 0.01;
    for (size_t f = 0; f < freq->size(); ++f) {
        BOOST_CHECK_CLOSE(atten(0, 0)(f), value, 1e-6);
        value *= 10.0;
    }
}

/**
 * Compare values of the Thorp model to Table 7 in
 * Weinburg, "Generic Sonar Model", NUWC TD-5971D (1985).
 * Because GSM uses slightly different constants then
 * Jensen, et. al., we only expect the results to match
 * within 20% and only at 400 Hz and above.
 */
BOOST_AUTO_TEST_CASE( thorp_test_a ) {
    cout << "=== attenuation_test: thorp_test_a ===" << endl;

    // simple values for points and distance

    wposition points(1, 1);
    points.altitude(0, 0, -1000.0);

    matrix<double> distance(1, 1);
    distance(0, 0) = 1000.0;

    // compute attenuation

    seq_vector::csptr freq( new  seq_log(10.0, 2.0, 14));
    matrix < vector<double> > atten(1, 1);
    atten(0, 0).resize(freq->size());

    attenuation_thorp model;
    model.attenuation(points, freq, distance, &atten);

    // Generic Sonar Model values

    static double gsm_thorp[] = { 0.00006, 0.00017, 0.00047, 0.00134, 0.00379,
            0.01125, 0.03615, 0.08538, 0.16469, 0.38326, 1.19919, 4.16885,
            12.81169, 27.26378 };

    for (size_t f = 0; f < freq->size(); ++f) {
        cout << (*freq)(f) << "\t" << atten(0, 0)(f) << endl;
        if ((*freq)(f) > 400.0) {
            BOOST_CHECK_CLOSE(atten(0, 0)(f), gsm_thorp[f], 20.0);
        }
    }
}

/**
 * Reproduce the plot shown in Jensen, et al. Computational
 * Ocean Acoustics p37 Fig. 1.20
 */
BOOST_AUTO_TEST_CASE( thorp_test_b ) {
    cout << "=== attenuation_test: thorp_test_b ===" << endl;

    // simple values for points and distance

    wposition points(1, 1);
    points.altitude(0, 0, -1000.0);

    matrix<double> distance(1, 1);
    distance(0, 0) = 1000.0;

    // compute attenuation

    seq_vector::csptr freq( new seq_log(10.0, pow(2.0,0.1), 1e7));
    matrix < vector<double> > atten(1, 1);
    atten(0, 0).resize(freq->size());

    attenuation_thorp model;
    model.attenuation(points, freq, distance, &atten);

    const char* name = USML_TEST_DIR "/ocean/test/attenuation_plot.csv" ;
    std::ofstream os(name) ;
    cout << "writing tables to " << name << endl ;

    os << "freq,atten" << endl ;
    for (size_t f = 0; f < freq->size(); ++f) {
        os << (*freq)(f) << "," << atten(0, 0)(f) << endl ;
    }
}

/// @}

BOOST_AUTO_TEST_SUITE_END()
