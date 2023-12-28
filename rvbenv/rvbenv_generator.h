/**
 * @file rvbenv_generator.h
 * Computes reverberation envelopes from eigenverbs.
 */
#pragma once

#include <usml/managed/update_notifier.h>
#include <usml/rvbenv/rvbenv_collection.h>
#include <usml/sensors/sensor_pair.h>
#include <usml/threads/thread_task.h>
#include <usml/types/seq_vector.h>
#include <usml/usml_config.h>

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <cstddef>

namespace usml {
namespace rvbenv {

using namespace usml::managed;
using namespace usml::sensors;
using namespace usml::threads;
using namespace usml::types;

/// @ingroup rvbenv
/// @{

/**
 * Background task to compute reverberation envelope collection for a bistatic
 * pair. Loops through the bistatic eigenverbs in the pair, computes the beam
 * pattern gain for each frequency/beam number, and asks rvbenv_collection to
 * add this contribution to the envelope time series. Repeats for each interface
 * in the ocean to incorporate the effects of the bottom, surface, and volume
 * reverberation. Notifies update listeners when the computation is complete.
 */
class USML_DECLSPEC rvbenv_generator
    : public thread_task,
      public update_notifier<rvbenv_collection::csptr> {
   public:
    /**
     * Initialize model with data from a sensor_pair.
     *
     * @param source      	Reference to the source for this pair.
     * @param receiver    	Reference to the receiver for this pair.
     * @param biverbs		Overlap of source and receiver eigenverbs.
     * @param frequencies  	Frequencies at which reverb is computed (Hz).
     * @param travel_times 	Times at which reverb is computed (sec).
     */
    rvbenv_generator(const sensor_model::sptr& source,
                     const sensor_model::sptr& receiver,
					 const biverb_collection::csptr& biverbs,
                     const seq_vector::csptr& frequencies,
                     const seq_vector::csptr& travel_times);

    /**
     * Compute reverberation envelope collection for a bistatic pair.
     * Loops through all of the bistatic eigenverbs in the pair and computes its
     * contribution to each source and receiver beam as a function of transmit
     * frequency and travel time.
     */
    virtual void run();

   private:
    /**
     * Computes the source beam gain as a function of DE and AZ for each
     * frequency and beam number. Rotates the DE and AZ of each eigenverb into
     * the coordinate system of the sensor.
     *
     * @param collection 	Source of information on sensor.
     * @param de		 	Elevation angle in world coordinates.
     * @param az		 	Azimuthal angle in world coordinates.
     * @param beam_work  	Temporary work space to compute beam paterns.
     * @param beam 		 	Beam gains in this direction
     * (rows=freq,cols=beam#).
     */
    void beam_gain_src(const rvbenv_collection* collection, double de,
                       double az, vector<double>& beam_work,
                       matrix<double>& beam);

    /**
     * Computes the receiver beam gain as a function of DE and AZ for each
     * frequency and beam number. Rotates the DE and AZ of each eigenverb into
     * the coordinate system of the sensor.
     *
     * @param collection 	Source of information on sensor.
     * @param de			Elevation angle in world coordinates.
     * @param az			Azimuthal angle in world coordinates.
     * @param beam_work 	Temporary work space to compute beam paterns.
     * @param beam 			Beam gains in this direction
     * (rows=freq,cols=beam#).
     */
    void beam_gain_rcv(const rvbenv_collection* collection, double de,
                       double az, vector<double>& beam_work,
                       matrix<double>& beam);

   private:
    // Reference to source sensor
    const sensor_model::sptr _source;

    // Reference to receiver sensor
    const sensor_model::sptr _receiver;

    /// Overlap of source and receiver eigenverbs.
    biverb_collection::csptr _biverbs;

    /// Frequencies at which reverb is computed (Hz).
    const seq_vector::csptr _frequencies;

    /// Times at which reverb is computed (sec).
    const seq_vector::csptr _travel_times;
};

/// @}
}  // end of namespace rvbenv
}  // end of namespace usml
