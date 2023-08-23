/**
 * @file rvbenv_generator.h
 * Computes reverberation envelopes from eigenverbs.
 */
#pragma once

#include <usml/beampatterns/bp_model.h>
#include <usml/bistatic/bistatic_pair.h>
#include <usml/eigenverbs/eigenverb_collection.h>
#include <usml/managed/update_notifier.h>
#include <usml/ocean/ocean_model.h>
#include <usml/platforms/sensor_model.h>
#include <usml/rvbenv/rvbenv_collection.h>
#include <usml/threads/read_write_lock.h>
#include <usml/threads/thread_task.h>
#include <usml/types/seq_vector.h>
#include <usml/usml_config.h>

#include <boost/numeric/ublas/matrix.hpp>
#include <cstddef>
#include <list>
#include <memory>

namespace usml {
namespace rvbenv {

using namespace usml::bistatic;
using namespace usml::managed;
using namespace usml::ocean;
using namespace usml::threads;

/// @ingroup rvbenv
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
 * updates its eigenverbs. If an existing rvbenv_generator is running for
 * this bistatic_pair, that task is aborted before the new rvbenv_generator
 * is created.
 */
class USML_DECLSPEC rvbenv_generator
    : public thread_task,
      public update_notifier<rvbenv_collection::csptr> {
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
    rvbenv_generator(bistatic_pair* pair, size_t num_azimuths);

    /**
     * Virtual destructor
     */
    virtual ~rvbenv_generator() {}

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
     * Computes the beam pattern for each source beam in this sensor.
     *
     * @param sensor 		Sensor for which to compute beam gain.
     * @param frequencies   List of frequencies to compute beam level for.
     * @param de			Elevation angle in world coordinates.
     * @param az			Azimuthal angle in world coordinates.
     */
    static matrix<double> beam_gain_src(const sensor_model::sptr& sensor,
                                        const seq_vector::csptr& frequencies,
                                        double de, double az);

    /**
     * Computes the beam pattern for each receiver beam in this sensor.
     *
     * @param sensor 		Sensor for which to compute beam gain.
     * @param frequencies   List of frequencies to compute beam level for.
     * @param de			Elevation angle in world coordinates.
     * @param az			Azimuthal angle in world coordinates.
     */
    static matrix<double> beam_gain_rcv(const sensor_model::sptr& sensor,
                                        const seq_vector::csptr& frequencies,
                                        double de, double az);

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

    /** Start time offset used to calculate envelope data. */
    double _initial_time;

    /** Ocean data to use for the envelope calculation. */
    ocean_model::csptr _ocean;

    /**
     * The sensor pair that instantiated this class
     */
    bistatic_pair* _bistatic_pair;

    /**
     * Source Beam Pattern List.
     */
    std::list<bp_model::csptr> _src_beam_list;

    /**
     * Receiver Beam Pattern List.
     */
    std::list<bp_model::csptr> _rcv_beam_list;

    /**
     * Interface collisions for wavefront emanating from the source.
     */
    eigenverb_collection::csptr _src_eigenverbs;

    /**
     * Interface collisions for wavefront emanating from the receiver.
     */
    eigenverb_collection::csptr _rcv_eigenverbs;

    /**
     * Collection of envelopes generated by this calculation.
     */
    rvbenv_collection* _envelopes;

    /**
     * Collection of envelopes generated by this calculation.
     */
    rvbenv_collection::csptr _rvbenv_collection;
};

/// @}
}  // end of namespace rvbenv
}  // end of namespace usml
