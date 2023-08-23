/**
 * @file rvbenv_collection.h
 * Computes the reverberation envelope time series for all combinations of
 * receiver azimuth, source beam number, receiver beam number.
 */
#pragma once

#include <usml/eigenverbs/eigenverb_model.h>
#include <usml/rvbenv/rvbenv_model.h>
#include <usml/threads/read_write_lock.h>
#include <usml/types/seq_vector.h>
#include <usml/types/wposition1.h>
#include <usml/usml_config.h>

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <cstddef>
#include <memory>

namespace usml {
namespace rvbenv {

using namespace usml::eigenverbs;
using namespace usml::threads;
using namespace usml::types;

/// @ingroup rvbenv
/// @{

/**
 * Computes and stores the reverberation envelope time series for all
 * combinations of receiver azimuth, source beam number, receiver beam number.
 * Relies on rvbenv_model to calculate the actual time series for each
 * envelope frequency. Each envelope is stored as a matrix that represents
 * the results as a function of the sensor_pair's envelope frequency (rows)
 * and two-way travel time (columns).
 */
class USML_DECLSPEC rvbenv_collection {
   public:
    /**
     * Data type used for reference to a rvbenv_collection.
     */
    typedef std::shared_ptr<const rvbenv_collection> csptr;

    /**
     * Reserve memory in which to store results as a series of
     * nested dynamic arrays.
     *
     * @param envelope_freq     Frequencies at which envelopes are computed.
     * @param travel_time       Times at which the sensor_pair's
     *                          reverberation envelopes are computed (Hz).
     * @param threshold         Minimum intensity level for valid reverberation
     *                          contributions (linear units).
     * @param num_azimuths      Number of receiver azimuths in result.
     * @param num_src_beams     Number of source beams in result.
     * @param num_rcv_beams     Number of receiver beams in result.
     * @param source_id         ID of the source sensor.
     * @param receiver_id       ID of the receiver sensor.
     * @param src_position      The source position when eigenverbs obtained.
     * @param rcv_position      The receiver position when eigenverbs obtained.
     */
    rvbenv_collection(seq_vector::csptr  envelope_freq,
                      seq_vector::csptr  travel_time, double threshold,
                      size_t num_azimuths, size_t num_src_beams,
                      size_t num_rcv_beams, int source_id, int receiver_id,
                      wposition1 src_position, wposition1 rcv_position);

    /**
     * Delete dynamic memory in each of the nested dynamic arrays.
     */
    ~rvbenv_collection();

    /**
     * ID of the the source sensor used to generate results.
     */
    int source_id() const { return _source_id; }

    /**
     * ID of the the receiver sensor used to generate results.
     */
    int receiver_id() const { return _receiver_id; }

    /**
     * Frequencies at which the source and receiver eigenverbs are computed
     * (Hz).
     */
    seq_vector::csptr envelope_freq() const { return _envelope_freq; }

    /**
     * Times at which the sensor_pair's reverberation envelopes
     * are computed (sec).
     */
    seq_vector::csptr travel_time() const { return _travel_time; }

    /**
     * Minimum power level for valid reverberation contributions
     * (linear units).
     */
    double threshold() const { return _threshold; }

    /** Number of receiver azimuths in result. */
    size_t num_azimuths() const { return _num_azimuths; }

    /** Number of receiver azimuths in result. */
    size_t num_src_beams() const { return _num_src_beams; }

    /** Number of receiver azimuths in result. */
    size_t num_rcv_beams() const { return _num_rcv_beams; }

    /** Range from source to receiver . */
    double slant_range() const { return _slant_range; }

    /**
     * Gets the source position.
     *
     * @return  The source position
     */
    wposition1 source_position() const { return _source_position; }

    /**
     * Sets the source position
     *
     * @param  position The source position
     */
    void source_position(wposition1 position) { _source_position = position; }

    /**
     * Gets the receiver position.
     *
     * @return  The receiver position.
     */
    wposition1 receiver_position() const { return _receiver_position; }

    /**
     * Sets the receiver position.
     *
     * @param  position The receiver position.
     */
    void receiver_position(wposition1 position) {
        _receiver_position = position;
    }

    /**
     * Retrieves intensity time series for one combination of parameters.
     *
     * @param azimuth     Receiver azimuth number.
     * @param src_beam    Source beam number.
     * @param rcv_beam    Receiver beam number
     * @return            Reverberation intensity at each point the time series.
     *                    Each row represents a specific envelope frequency.
     *                    Each column represents a specific travel time.
     */
    const matrix<double>& envelope(size_t azimuth, size_t src_beam,
                                   size_t rcv_beam) const {
        read_lock_guard guard(_envelopes_mutex);
        return *_envelopes[azimuth][src_beam][rcv_beam];
    }

    /**
     * Sets the intensity time series for one combination of parameters.
     *
     * @param intensities Reverberation intensity at each point the time series.
     * @param azimuth     Receiver azimuth number.
     * @param src_beam    Source beam number.
     * @param rcv_beam    Receiver beam number
     */
    void envelope(const matrix<double>& intensities, size_t azimuth,
                  size_t src_beam, size_t rcv_beam) const {
        write_lock_guard guard(_envelopes_mutex);
        *_envelopes[azimuth][src_beam][rcv_beam] = intensities;
    }

    /**
     * Adds the intensity contribution for a single combination of source
     * and receiver eigenverbs.  Loops over source and receiver beams to
     * apply beam pattern to each contribution.
     *
     * @param src_verb  Eigenverb contribution from the source.
     * @param rcv_verb  Eigenverb contribution from the receiver.
     * @param src_beam  Source beam level at each envelope frequency (ratio).
     *                  Each row represents a specific envelope frequency.
     *                  Each column represents a beam number.
     * @param rcv_beam	Receiver beam level at each envelope frequency
     * 				    (ratio). Each row represents a specific
     *  				envelope frequency. Each column
     * represents a beam
     * #.
     * @param scatter   Scattering strength at each envelope frequency (ratio).
     * @param xs2       Square of the relative distance from the
     *                  receiver to the target along the direction
     *                  of the receiver's length.
     * @param ys2       Square of the relative distance from the
     *                  receiver to the target along the direction
     *                  of the receiver's width.
     */
    void add_contribution(const eigenverb_model::csptr& src_verb,
                          const eigenverb_model::csptr& rcv_verb,
                          const matrix<double>& src_beam,
                          const matrix<double>& rcv_beam,
                          const vector<double>& scatter, double xs2,
                          double ys2);

    /**
     * Updates the current rvbenv_collection
     * via dead_reckoning with the parameters provided.
     *
     * @param delta_time    The time amount to shift the envelopes
     * @param slant_range   The range in meters from the source and receiver.
     * @param prev_range    The previous range in meters for the source and
     *                        receiver at the the start of delta_time.
     */
    void dead_reckon(double delta_time, double slant_range, double prev_range);

    /**
     * Writes the envelope data to disk
     *
     * An example of the file format is given below.
     * <pre>
     *	netcdf envelope_basic {
     *	dimensions:
     *		azimuth = 1 ;
     *		src_beam = 1 ;
     *		rcv_beam = 1 ;
     *		frequency = 3 ;
     *		travel_time = 400 ;
     *	variables*:
     *		double threshold ;
     *			threshold:units = "dB" ;
     *		double frequency(frequency) ;
     *			frequency:units = "hertz" ;
     *		double travel_time(travel_time) ;
     *			travel_time:units = "seconds" ;
     *		double intensity(azimuth, src_beam, rcv_beam, frequency,
     *travel_time) ; intensity:units = "dB" ; data:
     *
     *	 threshold = 1e-30 ;
     *
     *	 frequency = 1000, 2000, 3000 ;
     *
     *	 travel_time = 0, 0.100000000000001, 0.200000000000001,
     *0.300000000000002,
     *
     *	 intensity =
     *	  -300, -300, -300, -300, -300, -300, -300, -300, -300, -300, -300,
     *-300, -300, -300, -300, -300, -300, -300, -300, -300, -300, -300, -300,
     *-300, -300, -300, -300, -300, -300, -300, -300, -300, -300, -300, -300,
     *-300, -300, -300, -300, -300, -300, -300, -300, -300, -300, -300, -300,
     *-300, -300, -300, -300, -300, -300, -300, -300, -300, -300, -300, -300,
     *-300, -300, -300, -300, -300, -300, -300, -300, -300, -300, -300, -300,
     *-300, -300, -300, -300, -300, -300, -300, -300, -114.734197163402,
     *		-110.304659797997, -106.048828644939, -101.966703704228,
     *		-98.0582849758646, -94.3235724598482, -90.7625661561789,
     *		-87.3752660648567, -84.1616721858817, -81.1217845192539,
     *		-78.2556030649732, -75.5631278230396, -73.0443587934532,
     *		-70.6992959762139, -68.5279393713218, -66.5302889787769,
     *		-64.7063447985791, -63.0561068307284, -61.5795750752249,
     *		-60.2767495320685, -59.1476302012593, -58.1922170827972,
     *		-57.4105101766823, -56.8025094829146, -56.3682150014939,
     *		-56.1076267324205, -56.0207446756942, -56.107568831315,
     *-56.368099199283, -56.8023357795981, -57.4102785722604, -58.1919275772698,
     *		-59.1472827946264, -60.2763442243301, -61.579111866381,
     *-63.055585720779, -64.7057657875242, -66.5296520666165, -68.527244558056,
     *		-70.6985432618426, -73.0435481779763, -75.5622593064572,
     *		-78.2546766472853, -81.1208002004605, -84.1606299659829,
     *		-87.3741659438524, -90.7614081340691, -94.3223565366329,
     *		-98.0570111515439, -101.965371978802, -106.047439018407,
     *		-110.30321227036, -300, -300, -300, -300, -300, -300, -300,
     *-300, -300,
     *	}
     * </pre>
     */
    void write_netcdf(const char* filename) const;

   private:
    /**
     * Frequencies at which the source and receiver eigenverbs overlap (Hz).
     * Frequencies at which envelope will be computed.
     */
    seq_vector::csptr _envelope_freq;

    /**
     * Times at which the sensor_pair's reverberation envelopes
     * are computed (sec).
     */
    seq_vector::csptr _travel_time;

    /**
     * Minimum power level for valid reverberation contributions
     * (linear units).
     */
    const double _threshold;

    /**
     * Number of receiver azimuths in result.
     */
    const size_t _num_azimuths;

    /**
     * Number of source beams in result.
     */
    const size_t _num_src_beams;

    /**
     * Number of receiver beams in result.
     */
    const size_t _num_rcv_beams;

    /**
     * The slant range (in meters) of the sensor when the eigenverbs were
     * obtained.
     */
    double _slant_range;

    /**
     * ID for the source sensor
     */
    const int _source_id;

    /**
     * ID for the sensor sensor
     */
    const int _receiver_id;

    /**
     * The position of the source sensor when the eigenverbs were obtained.
     */
    wposition1 _source_position;

    /**
     * The position of the receiver sensor when the eigenverbs were obtained.
     */
    wposition1 _receiver_position;

    /**
     * Engine for computing Gaussian envelope contributions.
     */
    rvbenv_model _rvbenv_model;

    /**
     * Reverberation envelopes for each combination of parameters.
     * The order of indices is azimuth number, source beam number,
     * and then receiver beam number.  Each envelope is stored as a
     * matrix that represents the results a function of the sensor_pair's
     * envelope frequency (rows) and two-way travel time (columns).
     */
    matrix<double>**** _envelopes;

    /**
     * Mutex that locks during envelopes access
     */
    mutable read_write_lock _envelopes_mutex;
};

/// @}
}  // namespace rvbenv
}  // end of namespace usml
