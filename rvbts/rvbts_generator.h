/**
 * @file rvbts_generator.h
 * Background task to compute reverberation time series for a bistatic pair.
 */
#pragma once

#include <usml/biverbs/biverb_collection.h>
#include <usml/managed/managed_obj.h>
#include <usml/managed/update_notifier.h>
#include <usml/rvbts/rvbts_collection.h>
#include <usml/sensors/sensor_model.h>
#include <usml/sensors/sensor_pair.h>
#include <usml/threads/thread_task.h>
#include <usml/transmit/transmit_model.h>
#include <usml/types/orientation.h>
#include <usml/types/seq_vector.h>
#include <usml/types/wposition1.h>
#include <usml/usml_config.h>

#include <boost/numeric/ublas/matrix.hpp>

namespace usml {
namespace rvbts {

using namespace usml::managed;
using namespace usml::sensors;
using namespace usml::threads;
using namespace usml::transmit;
using namespace usml::types;

/// @ingroup rvbts
/// @{

/**
 * Background task to compute reverberation time series for a bistatic
 * pair.
 * Notifies update listeners when the computation is
 * complete.
 */
class USML_DECLSPEC rvbts_generator
    : public thread_task,
      public update_notifier<rvbts_collection::csptr> {
   public:
    /**
     * Initialize generator with state of sensor_pair at this time. Makes copies
     * of the position, orientation, speed, transmit pulses, and bistatic
     * eigenverbs at the time that the generator is constructed to ensure that
     * the state of the sensor pair is consistent throughout the calculation.
     *
     * @param pair       	Object to notify when complete.
     * @param source      	Reference to the source for this pair.
     * @param receiver    	Reference to the receiver for this pair.
     * @param treverb		Time increment for reverberation time series.
     * @param biverbs		Overlap of source and receiver eigenverbs.
     */
    rvbts_generator(const sensor_pair::sptr& pair,
                    const sensor_model::sptr& source,
                    const sensor_model::sptr& receiver,
					const double treverb,
                    const biverb_collection::csptr& biverbs);

    /**
     * Compute reverberation time series for a bistatic pair. Loops through all
     * of the bistatic eigenverbs in the pair and computes their contribution to
     * each receiver channel as a function of travel time.
     */
    virtual void run();

   private:
    /**
     * Compute source steerings for each transmit waveform. Steerings in the
     * transmission schedule are defined relative to the orientation of the host
     * platform. But, the beam patterns need them to be specified in array
     * coordinates. This implementation uses the orientation of the host to
     * convert the ordered heading into world coordinates. Then it uses the
     * orientation of the array to covert from world to array coordinates.
     *
     * Receiver beam patterns are less work because their steering directions
     * are defined relative to the array and not the array's host platform.
     */
    matrix<double> compute_src_steering() const;

    /// Human readable name for this object instance.
    const std::string _description;

    /// Reference to source sensor
    const sensor_model::sptr _source;

    /// Source position at time that class constructed.
    const wposition1 _source_pos;

    /// Source orientation at time that class constructed.
    const orientation _source_orient;

    /// Source speed at time that class constructed (m/s).
    const double _source_speed;

    /// List of transmit pulses for this source.
    transmit_list _transmit_schedule;

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

    /// Overlap of source and receiver eigenverbs.
    const biverb_collection::csptr _biverbs;

    /**
     * Source steerings relative to source array orientation. The rows represent
     * front, right, and up coordinates.  There is a column for each pulse in
     * the transmit schedule.
     */
    matrix<double> _source_steering;
};

/// @}
}  // end of namespace rvbts
}  // end of namespace usml
