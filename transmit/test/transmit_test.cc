/**
 * @example transmit/test/transmit_test.cc
 */

#include <usml/transmit/transmit.h>

#include <boost/test/unit_test.hpp>
#include <fstream>

BOOST_AUTO_TEST_SUITE(transmit_test)

using namespace usml::transmit;

/**
 * @ingroup transmit_test
 * @{
 */

/**
 * Test the ability to create a complex basebanded time series.
 */
BOOST_AUTO_TEST_CASE(create_time_series) {
    cout << "=== transmit_test: create_time_series ===" << endl;
    const char* csvname = USML_TEST_DIR "/transmit/test/create_time_series.csv";

    std::string type1("CW");
    double duration = 1.0;
    double fcenter = 10100.0;
    double delay = 0.0;
    double source_level = 1.0;
    transmit_cw transmit1(type1, duration, fcenter, delay, source_level,
                          window::type::HAMMING);

    std::string type2("LFM");
    duration = 1.5;
    fcenter = 10200.0;
    double bandwidth = 200.0;
    delay = 1.0;
    transmit_lfm transmit2(type1, duration, fcenter, bandwidth, delay,
                           source_level);

    // concatenate time series signals

    double fsample = 500.0;
    double fband = 10000.0;
    double phase = 0.0;

    cdvector data = transmit1.asignal(fsample, fband, phase, &phase);
    data += transmit1.asignal(fsample, fband, phase, &phase);
    size_t N = data.size();
    vector<double> time = seq_linear(0.0, 1.0 / fsample, N).data();

    // write timeseries data to disk for further analysis

    cout << "Saving data to " << csvname << endl;
    std::ofstream of(csvname);
    for (size_t n = 0; n < N; ++n) {
        of << time[n] << "," << data[n].real() << "," << data[n].imag() << endl;
    }
}

/// @}
BOOST_AUTO_TEST_SUITE_END()
