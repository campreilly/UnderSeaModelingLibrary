/**
 * @file rvbenv_collection.h
 * Computes the reverberation envelope time series for all combinations of
 * receiver azimuth, source beam number, and receiver beam number.
 */
#pragma once

#include <usml/biverbs/biverb_collection.h>
#include <usml/biverbs/biverb_model.h>
#include <usml/sensors/sensor_pair.h>
#include <usml/threads/read_write_lock.h>
#include <usml/types/orientation.h>
#include <usml/types/seq_vector.h>
#include <usml/types/wposition1.h>
#include <usml/usml_config.h>

#include <boost/numeric/ublas/matrix.hpp>
#include <cstddef>
#include <memory>

namespace usml {
namespace rvbenv {

using namespace usml::biverbs;
using namespace usml::sensors;
using namespace usml::threads;
using namespace usml::types;

/// @ingroup rvbenv
/// @{

/**
 * Computes and stores the reverberation envelope time series for all
 * combinations of receiver azimuth, source beam number, and receiver beam
 * number. Each envelope is stored as a matrix that represents
 * the results as a function of frequency (rows) and two-way travel time
 * (columns).
 */
class USML_DECLSPEC rvbenv_collection {
   public:
    /**
     * Data type used for reference to a rvbenv_collection.
     */
    typedef std::shared_ptr<const rvbenv_collection> csptr;

    /**
     * Initialize model with data from a sensor_pair.
     *
     * @param pair       	Bistatic pair that creates reverberation.
     * @param times 		Times at which reverb is computed (sec).
     * @param freqs  		Frequencies at which reverb is computed (Hz).
     * @param num_azimuths	Number of receiver azimuths in result.
     */
    rvbenv_collection(const sensor_pair::sptr& pair, seq_vector::csptr times,
                      seq_vector::csptr freqs, size_t num_azimuths);

    /**
     * Delete dynamic memory in each of the nested dynamic arrays.
     */
    ~rvbenv_collection();

    /// Mutex that locks object during access.
    read_write_lock& mutex() const { return _mutex; }

    /// Times at which reverberation is computed (sec).
    const seq_vector::csptr& times() const {
        read_lock_guard guard(mutex());
        return _times;
    }

    /// Frequencies at which reverberation is computed (Hz).
    const seq_vector::csptr& freqs() const {
        read_lock_guard guard(mutex());
        return _freqs;
    }

    /// Number of receiver azimuths in result.
    size_t num_azimuths() const {
        read_lock_guard guard(mutex());
        return _num_azimuths;
    }

    /// Number of source beam patterns
    size_t num_src_beams() const { return _num_src_beams; }

    /// Number of receiver beam patterns
    size_t num_rcv_beams() const { return _num_rcv_beams; }

    // Reference to source sensor
    const sensor_model::sptr source() const { return _source; }

    // Reference to receiver sensor
    const sensor_model::sptr receiver() const { return _receiver; }

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

    /// Bistatic eigenverbs at time that class constructed.
    const biverb_collection::csptr& biverbs() const {
        read_lock_guard guard(mutex());
        return _biverbs;
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
        read_lock_guard guard(mutex());
        return *_envelopes[azimuth][src_beam][rcv_beam];
    }

    /**
     * Adds the intensity contribution for a single bistatic eigenverb. Loops
     * over source and receiver beams to apply beam pattern to each
     * contribution.
     *
     * @param verb	   Bistatic eigenverb for envelope contribution.
     * @param src_beam Source gain for this eigenverb (rows=freq, cols=beam#).
     * @param rcv_beam Receiver gain for this eigenverb (rows=freq, cols=beam#).
     */
    void add_biverb(const biverb_model::csptr& verb,
                    const matrix<double>& src_beam,
                    const matrix<double>& rcv_beam);

    /**
     * Writes the envelope data to disk
     *
     * An example of the file format is given below.
     * <pre>

     *	}
     * </pre>
     */
    void write_netcdf(const char* filename) const;

   private:
    /// Mutex that locks object during access.
    mutable read_write_lock _mutex;

    /// Times at which reverberation is computed (sec).
    const seq_vector::csptr _times;

    /// Frequencies at which reverberation is computed (Hz).
    const seq_vector::csptr _freqs;

    /// Number of receiver azimuths in result.
    const size_t _num_azimuths;

    /// Number of source beam patterns
    size_t _num_src_beams;

    /// Number of receiver beam patterns
    size_t _num_rcv_beams;

    // Reference to source sensor
    sensor_model::sptr _source;

    // Reference to source sensor
    sensor_model::sptr _receiver;

    /// Source position at time that class constructed.
    wposition1 _source_pos;

    /// Receiver position at time that class constructed.
    wposition1 _receiver_pos;

    /// Source orientation at time that class constructed.
    orientation _source_orient;

    /// Receiver orientation at time that class constructed.
    orientation _receiver_orient;

    /// Source speed at time that class constructed.
    double _source_speed;

    /// Receiver speed at time that class constructed.
    double _receiver_speed;

    /// Bistatic eigenverbs at time that class constructed.
    biverb_collection::csptr _biverbs;

    /**
     * Reverberation envelopes for each combination of parameters.
     * The order of indices is azimuth number, source beam number,
     * and then receiver beam number.  Each envelope is stored as a
     * matrix that represents the results a function of the sensor_pair's
     * envelope frequency (rows) and two-way travel time (columns).
     */
    matrix<double>**** _envelopes;
};

/// @}
}  // namespace rvbenv
}  // end of namespace usml
