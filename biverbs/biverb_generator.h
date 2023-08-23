/**
 * @file biverb_generator.h
 * Computes reverberation envelopes from eigenverbs.
 */
#pragma once

#include <usml/bistatic/bistatic_pair.h>
#include <usml/biverbs/biverb_collection.h>
#include <usml/eigenverbs/eigenverb_collection.h>
#include <usml/eigenverbs/eigenverb_model.h>
#include <usml/managed/update_notifier.h>
#include <usml/ocean/ocean_model.h>
#include <usml/platforms/sensor_model.h>
#include <usml/threads/read_write_lock.h>
#include <usml/threads/thread_task.h>
#include <usml/types/seq_vector.h>
#include <usml/usml_config.h>

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <cstddef>
#include <memory>

namespace usml {
namespace biverbs {

using namespace usml::bistatic;
using namespace usml::managed;
using namespace usml::ocean;
using namespace usml::threads;

/// @ingroup biverbs
/// @{

/**
 * Computes reverberation envelopes from eigenverbs.
 * Combines eigenverbs to create the reverberation envelope,
 * as a function of travel time, frequency, and receiver beam number,
 * for a fixed set of azimuthal directions around each receiver.
 * An overlap is computed between each receiver eigenverb and
 * all of the source eigenverbs in its vicinity.  This overlap creates
 * a Gaussian reverberation envelope contribution in the time domain.
 *
 * The reverberation envelope contributions for each receiver azimuth
 * are incoherently power summed.  Beam patterns are applied to the
 * eigenverbs during envelope generation.  This allows a single pair of
 * eigenverbs to create separate envelopes for each receiver beam.
 * Eigenverbs and envelopes are computed as functions of frequency
 * so that the pre-computed reverberation results can be applied
 * to a variety of transmitted waveforms in the sonar training system.
 *
 * Invoked as a background thread_task by the bistatic_pair for
 * a specific source/receiver combination, whenever one of the sensors
 * updates its eigenverbs. If an existing biverb_generator is running for
 * this bistatic_pair, that task is aborted before the new biverb_generator
 * is created.
 */
class USML_DECLSPEC biverb_generator
    : public thread_task,
      public update_notifier<biverb_collection::csptr> {
   public:
    /**
     * Minimum intensity level for valid reverberation contributions (linear
     * units). Defaults to the linear equivalent of -300 dB.
     */
    static double intensity_threshold;

    /**
     * Maximum distance between the peaks of the source and receiver eigenverbs.
     * Specified as a ratio of distances relative to the receiver
     * eigenverb's length and width. Defaults to 6.0.
     */
    static double distance_threshold;

    /**
     * Initialize model parameters and reserve memory.
     *
     * @param pair       		Object to notify when complete.
     * @param num_azimuths      Number of receiver azimuths in result.
     */
    biverb_generator(bistatic_pair* pair, size_t num_azimuths);

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
     * envelopes.
     */
    virtual void run();

    /**
     * Time axis for all reverberation calculations.
     */
    static const seq_vector* travel_time() {
        read_lock_guard guard(_travel_time_mutex);
        return _travel_time.get();
    }

    /**
     * Resets time axis for all reverberation calculations.
     */
    static void travel_time(const seq_vector* time) {
        write_lock_guard guard(_travel_time_mutex);
        _travel_time.reset(time);
    }

    /**
     * Set to true when this task complete.
     */
    bool done() const { return _done; }

   private:
    /**
     * Create biverb from a single combination of source and receiver
     * eigneverbs.
     *
     * @param collection	Collection currently being updated.
     * @param ocean 		Reference to shared ocean for scattering strength.
     * @param interface		Interface number currently being processed.
     * @param scatter		Work space for scattering strength vs. frequency.
     * @param src_verb		Source eigenverb to be processed.
     * @param rcv_verb		Receiver eigenverb to be processed.
     */
    static void create_biverb(biverb_collection* collection,
                              const ocean_model::csptr& ocean, size_t interface,
                              vector<double>& scatter,
                              const eigenverb_model::csptr& src_verb,
                              const eigenverb_model::csptr& rcv_verb);

    /**
     * Compute the biverb that results from the overlap of two eigenverbs.
     *
     * @param src_verb		Source eigenverb to be processed.
     * @param rcv_verb		Receiver eigenverb to be processed.
     * @param xs2 			Square of source distance north of receiver.
     * @param ys2 			Square of source distance east of receiver.
     * @param scatter		Scattering strength vs. frequency.
     * @return 				Newly created bistatic eigenverb.
     */
    static biverb_model* compute_overlap(const eigenverb_model::csptr& src_verb,
                                         const eigenverb_model::csptr& rcv_verb,
                                         double xs2, double ys2,
                                         const vector<double>& scatter);

    /**
     * The mutex for static travel_time property.
     */
    static read_write_lock _travel_time_mutex;

    /**
     * Time axis for reverberation calculation.  Defaults to
     * a linear sequence out to 400 sec with a sampling period of 0.1 sec.
     */
    static std::unique_ptr<const seq_vector> _travel_time;

    /** Set to true when this task complete. */
    bool _done;

    /**
     * The sensor pair that instantiated this class
     */
    bistatic_pair* _bistatic_pair;

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
