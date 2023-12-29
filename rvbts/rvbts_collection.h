/**
 * @file rvbts_collection.h
 * Computes the reverberation envelope time series for all combinations of
 * transmit frequency, source beam number, and receiver beam number.
 */
#pragma once

#include <usml/biverbs/biverb_model.h>
#include <usml/sensors/sensor_model.h>
#include <usml/threads/read_write_lock.h>
#include <usml/types/orientation.h>
#include <usml/types/seq_vector.h>
#include <usml/types/wposition1.h>
#include <usml/usml_config.h>

#include <boost/numeric/ublas/matrix.hpp>
#include <cstddef>
#include <memory>

namespace usml {
namespace rvbts {

using namespace usml::biverbs;
using namespace usml::sensors;
using namespace usml::threads;
using namespace usml::types;

/// @ingroup rvbts
/// @{

/**
 * Computes and stores the reverberation envelope time series for all
 * combinations of source beam number, and receiver beam number. Each envelope
 * is stored as a matrix that represents the results as a function of frequency
 * (rows) and two-way travel time (columns).
 */
class USML_DECLSPEC rvbts_collection {
   public:
    /**
     * Data type used for reference to a rvbts_collection.
     */
    typedef std::shared_ptr<const rvbts_collection> csptr;

    /**
     * Initialize model with data from a sensor_pair.
     *
     * @param source      	Reference to the source for this pair.
     * @param receiver    	Reference to the receiver for this pair.
     * @param travel_times 	Times at which reverb is computed (sec).
     * @param frequencies  	Frequencies at which reverb is computed (Hz).
     */
    rvbts_collection(const sensor_model::sptr& source,
                      const sensor_model::sptr& receiver,
                      const seq_vector::csptr& travel_times,
                      const seq_vector::csptr& frequencies);

    /**
     * Delete dynamic memory in each of the nested dynamic arrays.
     */
    ~rvbts_collection();

    /// Mutex that locks object during access.
    read_write_lock& mutex() const { return _mutex; }

    /// Times at which reverberation is computed (sec).
    seq_vector::csptr travel_times() const { return _travel_times; }

    /// Frequencies at which reverberation is computed (Hz).
    seq_vector::csptr frequencies() const { return _frequencies; }

    // Reference to source sensor
    sensor_model::sptr source() const { return _source; }

    // Reference to receiver sensor
    sensor_model::sptr receiver() const { return _receiver; }

    /// Number of source beam patterns
    size_t num_src_beams() const { return _num_src_beams; }

    /// Number of receiver beam patterns
    size_t num_rcv_beams() const { return _num_rcv_beams; }

    /// Source position at time that class constructed.
    const wposition1& source_pos() const {
        read_lock_guard guard(mutex());
        return _source_pos;
    }

    /// Receiver position at time that class constructed.
    const wposition1& receiver_pos() const {
        read_lock_guard guard(mutex());
        return _receiver_pos;
    }

    /// Source orientation at time that class constructed.
    const orientation& source_orient() const {
        read_lock_guard guard(mutex());
        return _source_orient;
    }

    /// Receiver orientation at time that class constructed.
    const orientation& receiver_orient() const {
        read_lock_guard guard(mutex());
        return _receiver_orient;
    }

    /// Source speed at time that class constructed.
    double source_speed() const {
        read_lock_guard guard(mutex());
        return _source_speed;
    }

    /// Receiver speed at time that class constructed (m/s).
    double receiver_speed() const {
        read_lock_guard guard(mutex());
        return _receiver_speed;
    }

    /**
     * Envelope for one combination of source and receiver beam numbers.
     *
     * @param src_beam    Source beam number.
     * @param rcv_beam    Receiver beam number
     * @return            Reverberation intensity at each point the time series.
     *                    Each row represents a specific envelope frequency.
     *                    Each column represents a specific travel time.
     */
    const matrix<double>& envelope(size_t src_beam, size_t rcv_beam) const {
        read_lock_guard guard(_mutex);
        return *_envelopes[src_beam][rcv_beam];
    }

    /**
     * Adds the intensity contribution for a single bistatic eigenverb.
     * \f[
     *		G_{sr}(f,t) = \frac{ P(f) * B_s(f) * B_r(f) }{ T \sqrt{2\pi} }
     *					exp \left [ -\frac{ (t-\tau)^2 }{2 T^2} \right ]
     * \f]
     *
     * where
     * 	- \f$ P(f) \f$ = eigenverb power as function of frequency,
     * 	- \f$ B_s(f) \f$ = source beam level as function of frequency,
     * 	- \f$ B_r(f) \f$ = receiver beam level as function of frequency,
     * 	- \f$ T \f$ = eigenverb duration,
     * 	- \f$ \tau \f$ = arrival time of eigenverb peak,
     * 	- \f$ G_{sr}(f,t) \f$ = Reverb contribution vs. frequency and time.
     *
     * Loops over source beams, receiver beams, and frequencies to apply
     * eigenverb power and beam patterns to each contribution.
     *
     * @param verb	   Bistatic eigenverb for envelope contribution.
     * @param src_beam Source gain for this eigenverb (rows=freq, cols=beam#).
     * @param rcv_beam Receiver gain for this eigenverb (rows=freq, cols=beam#).
     *
     * TODO Add ability to adjust source and receiver beam steering.
     * TODO Add ability to incorporate Doppler shif for each contribution.
     */
    void add_biverb(const biverb_model::csptr& verb,
                    const matrix<double>& src_beam,
                    const matrix<double>& rcv_beam);

    /**
     * Writes reverberation envelope data to disk.
     *
     * An example of the file format is given below.
     * <pre>
     * </pre>
     */
    void write_netcdf(const char* filename) const;

   private:
    /// Mutex that locks object during access.
    mutable read_write_lock _mutex;

    /// Times at which reverberation is computed (sec).
    const seq_vector::csptr _travel_times;

    /// Frequencies at which reverberation is computed (Hz).
    const seq_vector::csptr _frequencies;

    // Reference to source sensor
    const sensor_model::sptr _source;

    // Reference to receiver sensor
    const sensor_model::sptr _receiver;

    /// Number of source beam patterns
    const size_t _num_src_beams;

    /// Number of receiver beam patterns
    const size_t _num_rcv_beams;

    /// Source position at time that class constructed.
    const wposition1 _source_pos;

    /// Receiver position at time that class constructed.
    const wposition1 _receiver_pos;

    /// Source orientation at time that class constructed.
    const orientation _source_orient;

    /// Receiver orientation at time that class constructed.
    const orientation _receiver_orient;

    /// Source speed at time that class constructed.
    const double _source_speed;

    /// Receiver speed at time that class constructed.
    const double _receiver_speed;

    /**
     * Reverberation envelopes for each source and receiver beam number.
     * Each envelope is stored as a uBLAS matrix that represents the results a
     * function of the sensor_pair's envelope frequency (rows) and two-way
     * travel time (columns).
     */
    matrix<double>*** _envelopes;
};

/// @}
}  // namespace rvbts
}  // end of namespace usml
