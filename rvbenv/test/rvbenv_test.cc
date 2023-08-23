/**
 * @example rvbenv/test/rvbenv_test.cc
 */

#include <usml/eigenverbs/eigenverb_collection.h>
#include <usml/eigenverbs/eigenverb_model.h>
#include <usml/rvbenv/rvbenv_collection.h>
#include <usml/types/seq_linear.h>
#include <usml/types/seq_vector.h>
#include <usml/types/wposition1.h>
#include <usml/ublas/math_traits.h>

#include <algorithm>
#include <boost/geometry/geometry.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/vector_expression.hpp>
#include <boost/test/unit_test.hpp>
#include <cmath>
#include <complex>
#include <cstddef>
#include <iostream>
#include <sstream>
#include <string>

BOOST_AUTO_TEST_SUITE(rvbenv_test)

using namespace usml::eigenverbs;
using namespace usml::rvbenv;
using namespace usml::types;
using namespace usml::ublas;

static const double c0 = 1500.0;  // constant sound speed

/**
 * @ingroup rvbenv_test
 * @{
 */

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
 * \f[
 *		I_{monostatic} = \frac{ 0.5 T_0 E_s^2 \sigma }
 *							  { Tsr \sqrt{ 4 L_s^2 W_s^2 }
 *} \f] Writes reverberation envelopes to the rvbenv_basic.nc files.
 */
BOOST_AUTO_TEST_CASE(rvbenv_basic) {
    cout << "=== envelope_test: rvbenv_basic ===" << endl;
    const char *ncname = USML_TEST_DIR "/waveq3d/test/rvbenv_basic.nc";

    // setup scenario for 30 deg D/E in 1000 meters of water

    double angle = M_PI / 6.0;
    double depth = 1000.0;
    double range = sqrt(3.0) * depth / (1852.0 * 60.0);
    double power = 0.2;

    // build a simple eigenverb

    auto *verb(new eigenverb_model);
    verb->time = 0.0;
    verb->position = wposition1(range, 0.0, -depth);
    verb->direction = 0.0;
    verb->grazing = angle;
    verb->sound_speed = c0;
    verb->de_index = 0;
    verb->az_index = 0;
    verb->source_de = -angle;
    verb->source_az = 0.0;
    verb->surface = 0;
    verb->bottom = 0;
    verb->caustic = 0;
    verb->upper = 0;
    verb->lower = 0;

    seq_vector::csptr freq(new seq_linear(1000.0, 1000.0, 3));
    verb->frequencies = freq;
    verb->power = vector<double>(freq->size(), power);
    verb->length = 20.0;
    verb->width = 10.0;

    // construct an rvbenv_collection

    seq_vector::csptr travel_time(new seq_linear(0.0, 0.01, 40.0));
    rvbenv_collection envelopes(freq,                   // envelope_freq
                                  travel_time,            // travel_time
                                  1e-30,                  // threshold
                                  1,                      // num_azimuths
                                  1,                      // num_src_beams
                                  1,                      // num_rcv_beams
                                  1,                      // source_id
                                  1,                      // receiver id
                                  wposition1(0.0, 0.0),   // source position
                                  wposition1(0.0, 0.0));  // receiver position

    vector<double> scatter(freq->size());
    matrix<double> src_beam(freq->size(), 1, 1.0);
    matrix<double> rcv_beam(freq->size(), 1, 1.0);
    for (int f = 0; f < freq->size(); ++f) {
        scatter[f] = 0.1 + 0.01 * f;
    }

    // add contributions at t=10 and t=30 sec

    eigenverb_model::csptr verb_csptr(verb);
    verb->time = 5.0;
    envelopes.add_contribution(verb_csptr, verb_csptr, src_beam, rcv_beam,
                               scatter, 0.0, 0.0);

    verb->time = 15.0;
    verb->power *= 0.5;
    envelopes.add_contribution(verb_csptr, verb_csptr, src_beam, rcv_beam,
                               scatter, 0.0, 0.0);

    auto index = (size_t) round(verb->time*2*100); // index of second peak

    cout << "writing to " << ncname << endl;
    envelopes.write_netcdf(ncname);

    // compare intensity to analytic solution for monostatic result (eqn.31).
    // - divides total energy by duration to estimate peak
    // - note that (Ls2+Lr2)*(Ws2+Wr2) = 2 Ls2 Ws2 when s=r
    // - includes extra 4 pi in denominator, based on Matlab results

    double factor = cos(angle) / c0;
    double sigma2 = verb->length * verb->length / 2.0;
    double duration = 0.5 * factor * sqrt(sigma2);
    vector<double> theory =
        10.0 * log10(0.25 * 0.5 * power * power * scatter /
                     (2.0 * verb->length * verb->width) / duration);
    cout << "duration=" << duration << endl;

    auto env = envelopes.envelope(0, 0, 0);
    cout << env << endl;
    for (size_t f = 0; f < freq->size(); ++f) {
        double model = 10.0 * log10(env(f, index));
        cout << "model=" << model << " theory=" << theory[f] << endl;
        BOOST_CHECK_SMALL(abs(model - theory[f]), 1e-4);
    }
}

/**
 * Test the ability to find eigenverbs generated from eigenverb_accuracy
 * test into a boost rtree and query them with an expected result.
 */
BOOST_AUTO_TEST_CASE(find_eigenverbs) {
    cout << "=== envelope_test: find_eigenverbs ===" << endl;
    const char *ncname = USML_TEST_DIR "/waveq3d/test/eigenverb_accuracy_";

    // interface to boost::geometry packages

    namespace bg = boost::geometry;
    namespace bgm = boost::geometry::model;
    typedef bgm::point<double, 2, bg::cs::cartesian> point;
    typedef bgm::box<point> box;

    // read eigenverbs for each interface from their own disk file

    int interfaces = 4;
    eigenverb_collection collection(interfaces);
    for (size_t n = 0; n < interfaces; ++n) {
        std::stringstream filename;
        filename << ncname << n << ".nc";
        collection.read_netcdf(filename.str().c_str(), n);
    }

    // use receiver eigenverbs lat, long, length and width
    // to create a bounding box.

    double rcv_verb_length = 200.0;       // meters
    double rcv_verb_width = 200.0;        // meters
    double rcv_verb_latitude = 45.0;      // north
    double rcv_verb_longitude = -45.0;    // east
    double lat_scaler = (60.0 * 1852.0);  // meters/degree

    double q = max(rcv_verb_length, rcv_verb_width);
    double latitude = rcv_verb_latitude;
    double longitude = rcv_verb_longitude;
    double delta_lat = q / lat_scaler;
    double delta_long = q / (lat_scaler * cos(to_radians(latitude)));

    // create a box, first point bottom left, second point upper right

    eigenverb_collection::box query_box(
        eigenverb_collection::point(latitude - delta_lat,
                                    longitude - delta_long),
        eigenverb_collection::point(latitude + delta_lat,
                                    longitude + delta_long));
    cout << "spatial query box:" << endl;
    cout << bg::wkt<box>(query_box) << endl;
    eigenverb_list found =
        collection.find_eigenverbs(query_box, eigenverb_model::BOTTOM);

    cout << "spatial found boxes:" << endl;
    for (const auto &verb : found) {
        eigenverb_collection::box found_box(
            eigenverb_collection::point(verb->bounding_box.south,
                                        verb->bounding_box.west),
            eigenverb_collection::point(verb->bounding_box.north,
                                        verb->bounding_box.east));
        cout << bg::wkt<box>(found_box) << endl;
    }
}

/// @}
BOOST_AUTO_TEST_SUITE_END()
