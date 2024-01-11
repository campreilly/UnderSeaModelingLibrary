/**
 * @example beampatterns/test/bp_test.cc
 */
#include <usml/beampatterns/beampatterns.h>

#include <boost/numeric/ublas/assignment.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/timer/timer.hpp>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>

BOOST_AUTO_TEST_SUITE(beampattern_test)

using namespace boost::unit_test;
using namespace usml::beampatterns;

const double freq = 900.0;
const double sound_speed = 1500.0;
const double spacing = (sound_speed / 1000.0) / 2.0;

/**
 * pretty print boost matrix
 */
void pretty_print_matrix(const matrix<double>& m) {
    for (unsigned i = 0; i < m.size1(); ++i) {
        cout << "| ";
        for (unsigned j = 0; j < m.size2(); ++j) {
            cout << std::fixed << std::setprecision(2) << m(i, j) << " | ";
        }
        cout << "|" << endl;
    }
    cout << std::scientific;
}

/**
 * Tests model accuracy by comparing two beam patterns. Computes beam level at
 * all DE and AZ angles, at a 1 deg spacing. Compares beam pattern to an ideal
 * pattern at each combination of DE and AZ, and throws an error if they
 * differ by more than 1e-10 (linear intensity gain). It then compares their
 * calculation of directivity gain.  Because many of the DI calculations are
 * approximations, this test allows the caller to adjust the sensitivity
 * of the DI comparison.
 *
 * Writes beam pattern to CSV file.  Each row in this file represents
 * beam levels as a function AZ from 0 to 360 degrees at a fixed DE.
 * The rows represent DE values from -90 to +90.
 *
 * @param AZ        Azimuthal steering angle (deg).
 * @param DE        Depression/elevation steering angle (deg).
 * @param bp        Beam pattern to test.
 * @param bp_comp   Ideal beam pattern to compare this pattern to.
 * @param csvname   File to write bp out to.
 * @param dierr     Acceptable difference in directivity gain (dB).
 * @param lvlerr    Acceptable difference in beam level (dB ratio).
 */
void pattern_test_generic(double AZ, double DE, const bp_model& bp,
                          const bp_model& bp_comp, const char* csvname,
                          double dierr = 1e-6, double lvlerr = 1e-6) {
    cout << "Saving beam data to " << csvname << endl;
    std::ofstream of(csvname);

    seq_vector::csptr frequencies(new seq_linear(freq, 1.0, 1));
    vector<double> beam(frequencies->size(), 0.0);
    vector<double> beam_comp(frequencies->size(), 0.0);

    // compute steering angle

    bvector steering(DE, AZ);

    // loop through DE and AZ angles

    double maxdiff = 0.0;
    for (int az_deg = 0; az_deg <= 360; az_deg += 1) {
        for (int de_deg = -90; de_deg <= 90; de_deg += 1) {
            // compute arrival angles
            bvector arrival(de_deg, az_deg);

            // compute beam levels at this DE and AZ angle
            bp.beam_level(arrival, frequencies, &beam, steering);
            bp_comp.beam_level(arrival, frequencies, &beam_comp, steering);

            // write beam beam to CSV file
            of << beam(0);
            if (de_deg != 90) {
                of << ",";
            }

            // compare beam levels at each DE and AZ
            const double level = -10 * log10(beam(0));
            const double level_comp = -10 * log10(beam_comp(0));
            double diff = abs(level - level_comp);
            if (abs(level_comp) > 1e-10) {
                diff /= abs(level_comp);
            }
            if (diff > lvlerr && maxdiff == 0.0) {
                cout << "AZ=" << az_deg << " DE=" << de_deg
                     << " level=" << level << " level_comp=" << level_comp
                     << endl;
            }
            maxdiff = max(maxdiff, diff);
        }
        of << endl;
    }
    BOOST_CHECK_MESSAGE(maxdiff <= lvlerr, "maxdiff=" << maxdiff);

    // compare directivity indices

    vector<double> DG(frequencies->size(), 0.0);
    bp.directivity(frequencies, &DG, steering);
    DG = -10.0 * log10(DG);

    vector<double> DG_comp(frequencies->size(), 0.0);
    bp_comp.directivity(frequencies, &DG_comp, steering);
    DG_comp = -10.0 * log10(DG_comp);

    const double level = DG(0);
    const double level_comp = DG_comp(0);
    maxdiff = abs(level - level_comp);
    BOOST_CHECK_MESSAGE(maxdiff <= dierr, "DI=" << level
                                                << " comp=" << level_comp
                                                << " diff=" << maxdiff);
}

/**
 * @ingroup bp_test
 * @{
 */

/**
 * Test the ability to build planar array with 3 rings that each have
 * different radii, numbers of elements, and offsets. Compares the location
 * of each element to an analytic solution.
 */
BOOST_AUTO_TEST_CASE(bp_utils_ring) {
    cout << "=== beampattern_test: bp_utils_ring ===" << endl;

    matrix<double> elem_locs;

    vector<double> radii(3);
    radii <<= 1, 2, 3;

    vector<int> num_elements(3);
    num_elements <<= 1, 2, 4;

    vector<double> offsets(3);
    offsets <<= 0, M_PI / 2, 3 * M_PI / 4;

    bp_con_ring(radii, num_elements, offsets, &elem_locs);
    pretty_print_matrix(elem_locs);
    int ind = 0;

    BOOST_CHECK_SMALL(elem_locs(ind, 0) - 1, 0.01);
    BOOST_CHECK_SMALL(elem_locs(ind, 1) - 0, 0.01);
    BOOST_CHECK_SMALL(elem_locs(ind, 2) - 0, 0.01);
    ind++;

    BOOST_CHECK_SMALL(elem_locs(ind, 0) - 0, 0.01);
    BOOST_CHECK_SMALL(elem_locs(ind, 1) - 2, 0.01);
    BOOST_CHECK_SMALL(elem_locs(ind, 2) - 0, 0.01);
    ind++;

    BOOST_CHECK_SMALL(elem_locs(ind, 0) - 0, 0.01);
    BOOST_CHECK_SMALL(elem_locs(ind, 1) - -2, 0.01);
    BOOST_CHECK_SMALL(elem_locs(ind, 2) - 0, 0.01);
    ind++;

    BOOST_CHECK_SMALL(elem_locs(ind, 0) - -2.12, 0.01);
    BOOST_CHECK_SMALL(elem_locs(ind, 1) - 2.12, 0.01);
    BOOST_CHECK_SMALL(elem_locs(ind, 2) - 0, 0.01);
    ind++;

    BOOST_CHECK_SMALL(elem_locs(ind, 0) - 2.12, 0.01);
    BOOST_CHECK_SMALL(elem_locs(ind, 1) - 2.12, 0.01);
    BOOST_CHECK_SMALL(elem_locs(ind, 2) - 0, 0.01);
    ind++;

    BOOST_CHECK_SMALL(elem_locs(ind, 0) - 2.12, 0.01);
    BOOST_CHECK_SMALL(elem_locs(ind, 1) - -2.12, 0.01);
    BOOST_CHECK_SMALL(elem_locs(ind, 2) - 0, 0.01);
    ind++;

    BOOST_CHECK_SMALL(elem_locs(ind, 0) - -2.12, 0.01);
    BOOST_CHECK_SMALL(elem_locs(ind, 1) - -2.12, 0.01);
    BOOST_CHECK_SMALL(elem_locs(ind, 2) - 0, 0.01);
}

/**
 * Test the ability to build rectangular planar array with different numbers
 * of elements in the front and right direction. Compares the location of each
 * element to an analytic solution.
 */
BOOST_AUTO_TEST_CASE(bp_utils_rect) {
    cout << "=== beampattern_test: bp_utils_rect ===" << endl;

    matrix<double> elem_locs;

    bp_con_uniform(3, 0.5,  // 3 elements front/back, 0.5m apart
                   2, 1.5,  // 2 elements right/left, 1.5m apart
                   1, 0,    // no up elements
                   &elem_locs);
    pretty_print_matrix(elem_locs);
    int ind = 0;

    BOOST_CHECK_SMALL(elem_locs(ind, 0) - -0.5, 0.01);
    BOOST_CHECK_SMALL(elem_locs(ind, 1) - -.75, 0.01);
    BOOST_CHECK_SMALL(elem_locs(ind, 2) - 0, 0.01);
    ind++;

    BOOST_CHECK_SMALL(elem_locs(ind, 0) - 0, 0.01);
    BOOST_CHECK_SMALL(elem_locs(ind, 1) - -.75, 0.01);
    BOOST_CHECK_SMALL(elem_locs(ind, 2) - 0, 0.01);
    ind++;

    BOOST_CHECK_SMALL(elem_locs(ind, 0) - 0.5, 0.01);
    BOOST_CHECK_SMALL(elem_locs(ind, 1) - -.75, 0.01);
    BOOST_CHECK_SMALL(elem_locs(ind, 2) - 0, 0.01);
    ind++;

    BOOST_CHECK_SMALL(elem_locs(ind, 0) - -0.5, 0.01);
    BOOST_CHECK_SMALL(elem_locs(ind, 1) - .75, 0.01);
    BOOST_CHECK_SMALL(elem_locs(ind, 2) - 0, 0.01);
    ind++;

    BOOST_CHECK_SMALL(elem_locs(ind, 0) - 0, 0.01);
    BOOST_CHECK_SMALL(elem_locs(ind, 1) - .75, 0.01);
    BOOST_CHECK_SMALL(elem_locs(ind, 2) - 0, 0.01);
    ind++;

    BOOST_CHECK_SMALL(elem_locs(ind, 0) - 0.5, 0.01);
    BOOST_CHECK_SMALL(elem_locs(ind, 1) - .75, 0.01);
    BOOST_CHECK_SMALL(elem_locs(ind, 2) - 0, 0.01);
}

/**
 * Compares the bp_omni beam level and directivity to a bp_arb array with a
 * single element. Writes beam pattern to bp_omni.csv file.
 */
BOOST_AUTO_TEST_CASE(bp_omni_test) {
    cout << "=== beampattern_test: bp_omni_test ===" << endl;
    const char* csvname = USML_TEST_DIR "/beampatterns/test/bp_omni.csv";
    bp_omni omni;
    matrix<double> locs(1, 3);
    bp_con_uniform(1, 0.0, 1, 0.0, 1, 0.0, &locs);
    bp_arb arb(locs);
    pattern_test_generic(0.0, 0.0, omni, arb, csvname, 0.02);
}

/**
 * Compares the bp_trig beam level at the MRA to 1 and the null to the
 * specified level. Compares directivity gain to analytic solution.
 * Writes beam pattern to bp_cos.csv and bp_sin.csv file.  The bp_test.m
 * script uses mesh_bp() to plot the cosine and sine patterns from these files
 * onto a single plot.
 */
BOOST_AUTO_TEST_CASE(bp_trig_test) {
    cout << "=== beampattern_test: bp_trig_test ===" << endl;
    seq_vector::csptr frequencies(new seq_linear(freq, 1.0, 1));
    vector<double> level(frequencies->size(), 0.0);
    const double DG = 0.1 + 0.9 / 3.0;

    const char* csvname = USML_TEST_DIR "/beampatterns/test/bp_cos.csv";
    bp_trig cosine(bp_trig_type::cosine, -20.0);
    pattern_test_generic(0.0, 0.0, cosine, cosine,
                         csvname);  // just to write file
    bvector arrival1(1.0, 0.0, 0.0);
    bvector arrival2(0.0, 1.0, 0.0);
    bvector arrival3(0.0, 0.0, 1.0);
    cosine.beam_level(arrival1, frequencies, &level);
    BOOST_CHECK_CLOSE(level(0), 1.0, 1e-10);
    cosine.beam_level(arrival2, frequencies, &level);
    BOOST_CHECK_CLOSE(level(0), 0.1, 1e-10);
    cosine.beam_level(arrival3, frequencies, &level);
    BOOST_CHECK_CLOSE(level(0), 0.1, 1e-10);
    cosine.directivity(frequencies, &level);
    BOOST_CHECK_CLOSE(level(0), DG, 1e-10);

    csvname = USML_TEST_DIR "/beampatterns/test/bp_sin.csv";
    bp_trig sine(bp_trig_type::sine, -20.0);
    pattern_test_generic(0.0, 0.0, sine, sine, csvname);  // just to write file
    sine.beam_level(arrival2, frequencies, &level);
    BOOST_CHECK_CLOSE(level(0), 1.0, 1e-10);
    sine.beam_level(arrival1, frequencies, &level);
    BOOST_CHECK_CLOSE(level(0), 0.1, 1e-10);
    sine.beam_level(arrival3, frequencies, &level);
    BOOST_CHECK_CLOSE(level(0), 0.1, 1e-10);
    sine.directivity(frequencies, &level);
    BOOST_CHECK_CLOSE(level(0), DG, 1e-10);
}

/**
 * Compares the bp_line beam level and directivity to a bp_arb array with a
 * the same number of elements. Writes beam pattern to bp_vla.csv
 * and bp_hla.csv files.  The bp_test.m script uses mesh_bp() to plot
 * the VLA and HLA patterns from these files and compare them to an equivalent
 * display created by bp_line().  Note that because the element spacing is
 * slightly larger than half the wavelength, the HLA pattern has a smaller
 * lobe at 180 than it has at 0.
 */
BOOST_AUTO_TEST_CASE(bp_line_test) {
    cout << "=== beampattern_test: bp_line_test ===" << endl;
    seq_vector::csptr frequencies(new seq_linear(freq, 1.0, 1));
    vector<double> level(frequencies->size(), 0.0);
    const char* csvname;

    // vertical line array

    csvname = USML_TEST_DIR "/beampatterns/test/bp_vla.csv";
    bp_line vla(5, spacing, bp_line_type::VLA);
    matrix<double> vla_locs(5, 3);
    bp_con_uniform(1, 0.0, 1, 0.0, 5, spacing, &vla_locs);
    bp_arb vla_arb(vla_locs);
    pattern_test_generic(10.0, -20.0, vla, vla_arb, csvname, 0.02);

    // check accuracy of MRA

    bvector steering(20.0, 0.0);
    vla.beam_level(steering, frequencies, &level, steering);
    BOOST_CHECK_CLOSE(level(0), 1.0, 1e-10);

    // horizontal line array

    csvname = USML_TEST_DIR "/beampatterns/test/bp_hla.csv";
    bp_line hla(5, spacing, bp_line_type::HLA);
    matrix<double> hla_locs(5, 3);
    bp_con_uniform(5, spacing, 1, 0.0, 1, 0.0, &hla_locs);
    bp_arb hla_arb(hla_locs);
    pattern_test_generic(0.0, 0.0, hla, hla_arb, csvname, 0.06);
}

/**
 * Compares bp_planar the beam level and directivity to a bp_arb array with a
 * the same number of elements. Writes beam pattern to bp_planar.csv file.
 * Uses a different number of elements in the horizontal (7) and vertical (5)
 * direction to find errors in array orientation.
 * The bp_tests.m script compares an image creates by this CSV file to the
 * one created by bp_planar.m. The pattern that both generate has a band of
 * 6 sidelobes to the right/left and another band of 10 sidelobes in the
 * up/down direction. These sidelobes get smaller as they move farther from the
 * array normal. The main lobes are normalized to a peak of one at both an
 * angle of zero and 180 degrees.
 */
BOOST_AUTO_TEST_CASE(bp_planar_test) {
    cout << "=== bp_planar ===" << endl;
    const char* csvname = USML_TEST_DIR "/beampatterns/test/bp_planar.csv";

    bp_planar planar(5, spacing, 7, spacing, true);
    matrix<double> elem_locs(35, 3);
    bp_con_uniform(1, 0.0, 7, spacing, 5, spacing, &elem_locs);
    bp_arb arb(elem_locs, true);
    pattern_test_generic(10.0, -20.0, planar, arb, csvname, 0.6);
}

/**
 * Test the functionality of bp_piston.
 * The user must verify this pattern in matlab:
 * mesh_bp('bp_piston.csv', [], []);  title('C piston');
 */
BOOST_AUTO_TEST_CASE(bp_piston_test) {
    cout << "=== bp_piston ===" << endl;
    const char* csvname = USML_TEST_DIR "/beampatterns/test/bp_piston.csv";
    seq_vector::csptr frequencies(new seq_linear(freq, 1.0, 1));
    vector<double> level(frequencies->size(), 0.0);

    bp_piston piston(4.0 * sound_speed / freq, true);
    pattern_test_generic(0.0, 0.0, piston, piston, csvname);

    bvector arrival1(1.0, 0.0, 0.0);
    piston.beam_level(arrival1, frequencies, &level);
    BOOST_CHECK_CLOSE(level(0), 1.0, 1e-10);

    bvector arrival2(-1.0, 0.0, 0.0);
    piston.beam_level(arrival2, frequencies, &level);
    BOOST_CHECK_CLOSE(level(0), 0.0, 1e-10);
}

/**
 * Computes the beam pattern created by combining bp_line with bp_omni
 * and writes it to the bp_multi.csv file. The bp_tests.m script draws an image
 * created by this CSV file using mesh_bp(). The pattern is the same as that
 * of the VLA.
 */
BOOST_AUTO_TEST_CASE(bp_multi_test) {
    cout << "=== beampattern_test: bp_multi_test ===" << endl;
    const char* csvname = USML_TEST_DIR "/beampatterns/test/bp_multi.csv";

    bp_model::csptr vla(new bp_line(5, spacing, bp_line_type::VLA));
    bp_model::csptr omni(new bp_omni());
    std::list<bp_model::csptr> bp_list;
    bp_list.push_back(omni);
    bp_list.push_back(vla);
    bp_multi multi(bp_list);
    pattern_test_generic(10.0, -20.0, multi, *vla, csvname, 0.02);
}

/**
 * Computes the beam pattern created by bp_cardioid and writes it
 * to the bp_cardioid.csv file. The bp_tests.m script draws an image
 * created by this CSV file using mesh_bp(). The main lobes are normalized
 * to a peak of one at both an angle of zero and 180 degrees.
 */
BOOST_AUTO_TEST_CASE(bp_cardioid_test) {
    cout << "=== beampattern_test: bp_cardioid_test ===" << endl;
    const char* csvname = USML_TEST_DIR "/beampatterns/test/bp_cardioid.csv";
    bp_cardioid cardioid;
    pattern_test_generic(10.0, -20.0, cardioid, cardioid,
                         csvname);  // write file

    seq_vector::csptr frequencies(new seq_linear(freq, 1.0, 1));
    vector<double> level(frequencies->size(), 0.0);

    bvector arrival1(1.0, 0.0, 0.0);
    cardioid.beam_level(arrival1, frequencies, &level);
    BOOST_CHECK_CLOSE(level(0), 1.0, 1e-10);

    bvector arrival2(-1.0, 0.0, 0.0);
    cardioid.beam_level(arrival2, frequencies, &level);
    BOOST_CHECK_CLOSE(level(0), 0.0, 1e-10);

    // compare directivities to those in Butler/Sherman reference

    cardioid.directivity(frequencies, &level);
    double DI = -10.0 * log10(level(0));
    BOOST_CHECK_SMALL(DI - 4.8, 0.05);

    bp_cardioid super_cardioid(sqrt(3.0));
    super_cardioid.directivity(frequencies, &level);
    DI = -10.0 * log10(level(0));
    BOOST_CHECK_SMALL(DI - 5.7, 0.05);

    bp_cardioid hyper_cardioid(3.0);
    hyper_cardioid.directivity(frequencies, &level);
    DI = -10.0 * log10(level(0));
    BOOST_CHECK_SMALL(DI - 6.0, 0.05);
}

/**
 * Test the the weight function of bp_arb.
 * Compares a 5 element line array to a 7 element arb array
 * where the end elements have weights of 0.
 */
BOOST_AUTO_TEST_CASE(bp_arb_test) {
    cout << "=== beampattern_test: bp_arb_test ===" << endl;
    const char* csvname = USML_TEST_DIR "/beampatterns/test/bp_arb.csv";

    matrix<double> elem_locs(5, 3);
    elem_locs <<= 0.0, 0.0, -3.0 * spacing,  // uneven vertical line array
        0.0, 0.0, -1.0 * spacing, 0.0, 0.0, 0.0 * spacing, 0.0, 0.0,
        1.0 * spacing, 0.0, 0.0, 4.0 * spacing;
    bp_arb arb(elem_locs);
    pattern_test_generic(10.0, -20.0, arb, arb, csvname);  // just to write file
}

/**
 * Test the the weight function of bp_arb.
 * Compares a 5 element line array to a 7 element arb array
 * where the end elements have weights of 0.
 */
BOOST_AUTO_TEST_CASE(bp_arb_weight_test) {
    cout << "=== beampattern_test: bp_arb_weight_test ===" << endl;
    const char* csvname = USML_TEST_DIR "/beampatterns/test/bp_arb_weight.csv";

    bp_line vla(5, spacing, bp_line_type::VLA);
    matrix<double> elem_locs(7, 3);
    bp_con_uniform(1, 0.0, 1, 0.0, 7, spacing, &elem_locs);
    vector<double> weights(7);
    weights <<= 0, 1, 1, 1, 1, 1, 0;
    bp_arb arb(elem_locs, weights);
    pattern_test_generic(10.0, -20.0, arb, vla, csvname, 0.02);
}

/**
 * Test the ability to build beam pattern from data grid.
 */
BOOST_AUTO_TEST_CASE(bp_grid_test) {
    cout << "=== beampattern_test: bp_grid_test ===" << endl;

    // build a set of axes for this beam pattern

    seq_vector::csptr freq(new seq_linear(100.0, 100.0, 5));
    seq_vector::csptr de(new seq_linear(to_radians(-2.0), to_radians(1.0), 5));
    seq_vector::csptr az(new seq_linear(0, 1.0, 1));
    seq_vector::csptr axis[] = {freq, de, az};
    auto* grid = new gen_grid<3>(axis);

    // fill in a data vector

    double values[5 * 5 * 1] = {
            1.0,  0.75, 0.5,  0.75, 0.81,
            0.87, 0.75, 0.5,  0.75, 0.41,
            0.2,  0.75, 0.5,  0.75, 0.33,
            0.61, 0.75, 0.5,  0.75, 0.97,
            0.53, 0.75, 0.5,  0.75, 0.53};
    size_t index[3];
    int n = 0;
    for (int i = 0; i < axis[0]->size(); ++i) {
        for (int j = 0; j < axis[1]->size(); ++j) {
            index[0] = j;
            index[1] = i;
            index[2] = 0;
            grid->setdata(index, values[n++]);
        }
    }

    // create bp_model from this grid

    data_grid<3>::csptr grid_sptr(grid);
    bp_grid beam(grid_sptr);

    // test ability to extract data at DE=0

    vector<double> level(axis[0]->size(), 0.0);
    bvector arrival(1.0, 0.0, 0.0);
    beam.beam_level(arrival, axis[0], &level);
    n = 10;
    for (int f = 0; f < axis[0]->size(); ++f) {
        BOOST_CHECK_CLOSE(level[f], values[n++], 1e-5);
    }
}

/**
 * Test the ability to build a cookie cutter beam pattern.
 */
BOOST_AUTO_TEST_CASE(bp_solid_test) {
    cout << "=== beampattern_test: bp_solid_test ===" << endl;
    const char* csvname = USML_TEST_DIR "/beampatterns/test/bp_solid.csv";

    seq_vector::csptr frequencies(new seq_linear(freq, 1.0, 1));
    bp_solid solid(45.0, 90.0);
    pattern_test_generic(10.0, -20.0, solid, solid,
                         csvname);  // just to write file

    vector<double> level(frequencies->size(), 0.0);
    solid.directivity(frequencies, &level);
    level = -10.0 * log10(level);
    const double test = 10 * log10(16);
    BOOST_CHECK_CLOSE(level(0), test, 1e-10);
}

/// @}

BOOST_AUTO_TEST_SUITE_END()
