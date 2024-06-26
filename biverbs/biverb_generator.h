/**
 * @file biverb_generator.h
 * Background task to compute bistatic eigenverbs.
 */
#pragma once

#include <usml/biverbs/biverb_collection.h>
#include <usml/eigenverbs/eigenverb_collection.h>
#include <usml/managed/update_notifier.h>
#include <usml/sensors/sensor_pair.h>
#include <usml/threads/thread_task.h>
#include <usml/usml_config.h>

namespace usml {
namespace biverbs {

using namespace usml::eigenverbs;
using namespace usml::managed;
using namespace usml::sensors;
using namespace usml::threads;

/// @ingroup biverbs
/// @{

/**
 * Background task to compute bistatic eigenverbs. Automatically invoked by a
 * sensor_pair whenever one of the sensors updates its eigenverbs. If an
 * existing biverb_generator is running for this sensor_pair, that task is
 * aborted before the new background task is created. Results are stored in the
 * sensor_pair that invoked this background task, unless the task is aborted
 * prior to completion.
 */
class USML_DECLSPEC biverb_generator
    : public thread_task,
      public update_notifier<biverb_collection::csptr> {
   public:
    /**
     * Initialize model parameters and reserve memory. Note that passing the
     * src_eigenverbs and rcv_eigenverbs of the pair as their own arguments
     * allows the sensor_pair class to invoke this contructor while pair is
     * locked.
     *
     * @param pair       		Object to notify when complete.
     * @param src_eigenverbs 	Interface collisions for source.
     * @param rcv_eigenverbs 	Interface collisions for receiver.
     */
    biverb_generator(const sensor_pair::sptr& pair,
                     const eigenverb_collection::csptr& src_eigenverbs,
                     const eigenverb_collection::csptr& rcv_eigenverbs);

    /**
     * Virtual destructor
     */
    virtual ~biverb_generator() {}

    /**
     * Executes the Eigenverb reverberation model. For each receiver eigenverb,
     * it loops through the list of source eigenverbs looking for overlaps.
     *
     * First, it computes the great circle range and bearing of the source
     * relative to the receiver.  The combination is skipped if the location of
     * the source (its peak intensity) is more than three (3) times the
     * length/width of the receiver eigenverb,
     * Next, it computes the scattering strength and beam patterns for
     * this source/receiver combination.
     * Finally, it uses the evelope_collection.add_contribution() method
     * to add this this source/receiver combination to the reverberation
     * time series.
     */
    virtual void run();

   private:
    /**
     * The sensor pair that instantiated this class
     */
    sensor_pair::sptr _sensor_pair;

    /**
     * Interface collisions for wavefront emanating from the source. Stored at
     * the beginning so that they don't change in the middle of calculation.
     */
    eigenverb_collection::csptr _src_eigenverbs;

    /**
     * Interface collisions for wavefront emanating from the receiver. Stored at
     * the beginning so that they don't change in the middle of calculation.
     */
    eigenverb_collection::csptr _rcv_eigenverbs;

    /**
     * Collection of bistatic eigenverbs generated by this calculation.
     */
    biverb_collection::csptr _collection;
};

/// @}
}  // end of namespace biverbs
}  // end of namespace usml
