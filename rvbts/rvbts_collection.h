/**
 * @file rvbts_collection.h
 * Computes and stores reverberation time series for each receiver channel.
 */
#pragma once

#include <usml/biverbs/biverb_model.h>
#include <usml/sensors/sensor_model.h>
#include <usml/transmit/transmit_model.h>
#include <usml/types/orientation.h>
#include <usml/types/seq_vector.h>
#include <usml/types/wposition1.h>
#include <usml/usml_config.h>

#include <boost/numeric/ublas/matrix.hpp>
#include <memory>

namespace usml {
namespace rvbts {

using namespace usml::biverbs;
using namespace usml::sensors;
using namespace usml::threads;
using namespace usml::transmit;
using namespace usml::types;

/// @ingroup rvbts
/// @{

/**
 * Computes and stores reverberation time series for each receiver channel.
 *
 * This implementation supports beam level simulations where each receiver
 * channel has its own beam pattern and steering. It lacks the phase delay
 * between channels needed to support element level simulation.
 */
class USML_DECLSPEC rvbts_collection {
   public:
    /**
     * Data type used for reference to a rvbts_collection.
     */
    typedef std::shared_ptr<const rvbts_collection> csptr;

    /**
     * Initialize model parameters with state of sensor_pair at the time that
     * reverberation generator was created.
     *
     * @param source      	  Reference to source sensor
     * @param source_pos      Source position at this time.
     * @param source_orient   Source orientation at this time.
     * @param source_speed    Source speed at this time.
     * @param receiver        Reference to receiver sensor
     * @param receiver_pos    Receiver position at this time.
     * @param receiver_orient Receiver orientation at this time.
     * @param receiver_speed  Receiver speed at this time.
     * @param travel_times    Times at which reverberation is computed (sec).
     */
    rvbts_collection(const sensor_model::sptr& source,
                     const wposition1 source_pos,
                     const orientation& source_orient, const double source_speed,
                     const sensor_model::sptr& receiver,
                     const wposition1 receiver_pos,
                     const orientation& receiver_orient,
                     const double receiver_speed,
                     const seq_vector::csptr& travel_times);

    // Reference to source sensor.
    sensor_model::sptr source() const { return _source; }

    /// Source position at time that class constructed.
    const wposition1& source_pos() const { return _source_pos; }

    /// Source orientation at time that class constructed.
    const orientation& source_orient() const { return _source_orient; }

    /// Source speed at time that class constructed.
    double source_speed() const { return _source_speed; }

    // Reference to receiver sensor.
    sensor_model::sptr receiver() const { return _receiver; }

    /// Receiver position at time that class constructed.
    const wposition1& receiver_pos() const { return _receiver_pos; }

    /// Receiver orientation at time that class constructed.
    const orientation& receiver_orient() const { return _receiver_orient; }

    /// Receiver speed at time that class constructed (m/s).
    double receiver_speed() const { return _receiver_speed; }

    /// Receiver times at which reverberation is computed (sec).
    seq_vector::csptr travel_times() const { return _travel_times; }

    /**
     * Adds the intensity contribution for a single bistatic eigenverb.
     * \f[
     *		G_{sr}(f,t) = \frac{ P(f) * B_s(f) * B_r(f) }{ T \sqrt{2\pi} }
     *					exp \left [ -\frac{ (t-\tau)^2 }{2 T^2}
     *\right ] \f]
     *
     * where
     * 	- \f$ P(f) \f$ = eigenverb power as function of frequency,
     * 	- \f$ B_s(f) \f$ = source beam level as function of frequency,
     * 	- \f$ B_r(f) \f$ = receiver beam level as function of frequency,
     * 	- \f$ T \f$ = eigenverb duration,
     * 	- \f$ \tau \f$ = arrival time of eigenverb peak,
     * 	- \f$ G_{sr}(f,t) \f$ = Reverb contribution vs. frequency and time.
     *
     * Loops over receiver beams and adds the Gaussian contribution to each
     * channel. Interpolates eigenverb power to the transmit frequency. Applies
     * the source and receiver beam patterns to each eigenverb contribution.
     *
     * @param verb	   	Bistatic eigenverb for time series contribution.
     * @param transmit	Single waveform in a transmission schedule.
     * @param steering 	Transmit steering relative to source array.
     */
    void add_biverb(const biverb_model::csptr& verb,
                    const transmit_model::csptr& transmit, const bvector& steering);

    /**
     * Writes reverberation time series data to disk.
     *
     * An example of the file format is given below.
     * <pre>
     * </pre>
     */
    void write_netcdf(const char* filename) const;

   private:
    // Reference to source sensor
    const sensor_model::sptr _source;

    /// Source position at time that class constructed.
    const wposition1 _source_pos;

    /// Source orientation at time that class constructed.
    const orientation _source_orient;

    /// Host orientation at time that class constructed.
    const orientation _host_orient;

    /// Source speed at time that class constructed (m/s).
    const double _source_speed;

    /// Reference to receiver sensor
    const sensor_model::sptr _receiver;

    /// Receiver position at time that class constructed.
    const wposition1 _receiver_pos;

    /// Receiver orientation at time that class constructed.
    const orientation _receiver_orient;

    /// Receiver speed at time that class constructed (m/s).
    const double _receiver_speed;

    /// Receiver times at which reverberation is computed (sec).
    const seq_vector::csptr _travel_times;

    /// Reverberation time series for each receiver channel.
    matrix<double> _time_series;
};

/// @}
}  // namespace rvbts
}  // end of namespace usml
