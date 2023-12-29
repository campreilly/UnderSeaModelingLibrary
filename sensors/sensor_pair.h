/**
 * @file sensor_pair.h
 * Cache of modeling products for link between source and receiver.
 */
#pragma once

#include <usml/biverbs/biverb_collection.h>
#include <usml/eigenrays/eigenray_collection.h>
#include <usml/eigenverbs/eigenverb_collection.h>
#include <usml/managed/managed_obj.h>
#include <usml/managed/update_listener.h>
#include <usml/managed/update_notifier.h>
#include <usml/rvbts/rvbts_collection.h>
#include <usml/sensors/sensor_model.h>
#include <usml/threads/read_write_lock.h>
#include <usml/usml_config.h>
#include <usml/wavegen/wavefront_listener.h>

#include <list>
#include <memory>
#include <string>

namespace usml {
namespace biverbs {
class biverb_generator;
}
}  // namespace usml

namespace usml {
namespace rvbts {
class rvbts_generator;
}
}  // namespace usml

namespace usml {
namespace sensors {

using namespace usml::biverbs;
using namespace usml::eigenrays;
using namespace usml::eigenverbs;
using namespace usml::managed;
using namespace usml::rvbts;
using namespace usml::threads;
using namespace usml::wavegen;

/// @ingroup sensors
/// @{

/**
 * Cache of modeling products for link between source and receiver. Listens for
 * acoustic changes in its component sensor_models. Each eigenray represents a
 * single acoustic path between a source and target. The dirpaths are eigenrays
 * that connect this source and receiver; they represent the multipath direct
 * blast contributions to the received signal. Eigenverbs are a Gaussian beam
 * projection of an acoustic ray onto a reverberation interface at the point of
 * collision. The biverbs represent the bistatic overlap between the source and
 * receiver eigenverbs for this pair. Notifies sensor_pair update listeners
 * when all of the calculations are complete.
 */
class USML_DECLSPEC sensor_pair
    : public managed_obj<std::string, sensor_pair>,
      public wavefront_listener,
      public update_listener<biverb_collection::csptr>,
      public update_listener<rvbts_collection::csptr>,
      public update_notifier<sensor_pair> {
   public:
    /**
     * Construct link between source and receiver. Makes this pair an
     * update_listener to both the source and receiver. The source and receiver
     * will be equal for monostatic sensors.
     *
     * @param    source      Reference to the source for this pair.
     * @param    receiver    Reference to the receiver for this pair.
     */
    sensor_pair(const sensor_model::sptr& source,
                const sensor_model::sptr& receiver);

    /**
     * Virtual destructor.
     */
    virtual ~sensor_pair();

    /// Lookup key for this combination of source and receiver
    std::string hash_key() const {
        read_lock_guard guard(_mutex);
        return generate_hash_key(_source->keyID(), _receiver->keyID());
    }

    /// Reference to the source sensor.
    const sensor_model::sptr source() const {
        read_lock_guard guard(_mutex);
        return _source;
    }

    /// Reference to the receiving sensor.
    const sensor_model::sptr receiver() const {
        read_lock_guard guard(_mutex);
        return _receiver;
    }

    /// Direct paths that connect source and receiver locations.
    eigenray_collection::csptr dirpaths() const {
        read_lock_guard guard(_mutex);
        return _dirpaths;
    }

    /// Interface collisions for wavefront emanating from the source.
    eigenverb_collection::csptr rcv_eigenverbs() const {
        read_lock_guard guard(_mutex);
        return _rcv_eigenverbs;
    }

    /// Interface collisions for wavefront emanating from the receiver.
    eigenverb_collection::csptr src_eigenverbs() const {
        read_lock_guard guard(_mutex);
        return _src_eigenverbs;
    }

    /// Overlap of source and receiver eigenverbs.
    biverb_collection::csptr biverbs() const {
        read_lock_guard guard(_mutex);
        return _biverbs;
    }

    /// Reverberation time series time series.
    rvbts_collection::csptr rvbts() const {
    	read_lock_guard guard(_mutex);
        return _rvbts;
    }

    /// True if eigenverbs computed for this sensor.
    bool compute_reverb() const {
        read_lock_guard guard(_mutex);
        return _compute_reverb;
    }

    /**
     * Utility to generate a hash key for the bistatic_template
     *
     * @param    src_id   The source id used to generate the hash_key
     * @param    rcv_id   The receiver id used to generate the hash_key
     * @return   string   containing the generated hash_key.
     */
    static std::string generate_hash_key(int src_id, int rcv_id);

    /**
     * Queries for the bistatic pair for the complement of the given sensor.
     *
     * @param   sensor 	Reference to sensor that requested the complement.
     * @return  		Reference to complement sensor of the pair.
     */
    sensor_model::sptr complement(const sensor_model::sptr& sensor) const;

    /**
     * Notify this pair of eigenray and eigenverb changes for one of its
     * sensors. Updates the direct path eigenrays and bistatic eigenverbs for
     * this pair. Launches a new biverb_generator if this sensor_pair supports
     * reverberation and if both source and receiver eigenverbs exist once this
     * update is complete.
     *
     * This computation can be triggered by updates from either the source or
     * receiver object in this sensor_pair. If this is an update from a
     * bistatic receiver, then the sense of source and target is reversed for
     * the calculation of direct path bistatic eigenrays. This reversal is valid
     * if the eigenrays have source/receiver reciprocity, which they might not
     * have in complex environments because of accuracy limitations in the
     * wavefront modeling.
     *
     * @param sensor		Pointer to updated sensor.
     * @param eigenrays		Transmission loss results for this sensor
     * @param eigenverbs 	Reverberation results for this sensor.
     */
    virtual void update_wavefront_data(
        const sensor_model* sensor, eigenray_collection::csptr eigenrays,
        eigenverb_collection::csptr eigenverbs) override;

    /**
     * Update bistatic eigenverbs using results of biverb_generator.
     * Stores a reference to the bistatic eigenverbs then launches a new
     * rvbts_generator to compute reverberation time series.
     *
     * @param  object	Updated bistatic eigenverbs collection.
     */
    virtual void notify_update(const biverb_collection::csptr* object) override;

    /**
     * Update bistatic eigenverbs using results of rvbts_generator.
     * Stores a reference to the reverberation time series then notifies listeners
     * that this sensor_pair has been updated.
     *
     * @param  object	Updated reverberation time series collection.
     */
    virtual void notify_update(const rvbts_collection::csptr* object) override;

    /**
     * Notify listeners that this sensor_pair has been updated.
     *
     * @param object    Reference to the object that has been updated.
     */
    virtual void notify_update(const sensor_pair* object) const override;

   private:
    /// Mutex to that locks pair updates.
    mutable read_write_lock _mutex;

    /// Reference to the source sensor.
    /// The source and receiver will be equal for monostatic sensors.
    const sensor_model::sptr _source;

    /// Reference to the receiving sensor.
    /// The source and receiver will be equal for monostatic sensors.
    const sensor_model::sptr _receiver;

    /// Direct paths that connect source and receiver locations.
    eigenray_collection::csptr _dirpaths;

    /// True if computing reverberation for this pair.
    bool _compute_reverb{false};

    /// Interface collisions for wavefront emanating from the source.
    eigenverb_collection::csptr _src_eigenverbs;

    /// Interface collisions for wavefront emanating from the receiver.
    eigenverb_collection::csptr _rcv_eigenverbs;

    /// Overlap of source and receiver eigenverbs.
    biverb_collection::csptr _biverbs;

    /// Reverberation time series time series.
    rvbts_collection::csptr _rvbts;

    /// Background task used to generate biverb objects.
    std::shared_ptr<biverb_generator> _biverb_task;

    /// Background task used to generate reverberation time series objects.
    std::shared_ptr<rvbts_generator> _rvbts_task;
};

typedef std::list<sensor_pair::sptr> pair_list;

/// @}
}  // namespace sensors
}  // end of namespace usml
