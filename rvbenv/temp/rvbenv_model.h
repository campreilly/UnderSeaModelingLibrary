/**
 * @file rvbenv_model.h
 * Computes the reverberation envelope time series for a single combination of
 * receiver azimuth, source beam number, receiver beam number.
 */
#pragma once

#include <usml/eigenverbs/eigenverb_model.h>
#include <usml/types/seq_vector.h>
#include <usml/usml_config.h>

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <memory>

namespace usml {
namespace rvbenv {

using namespace usml::eigenverbs;
using namespace usml::types;

class rvbenv_collection;  // forward declaration

/// @ingroup rvbenv
/// @{

/**
 * Computes the reverberation envelope time series for a single combination of
 * receiver azimuth, source beam number, receiver beam number. The envelope is
 * stored as a matrix that represents the results a function of the
 * sensor_pair's envelope frequency (rows) and two-way travel time (columns).
 *
 * This implementation requires the receiver eigenverbs to be interpolated
 * onto the envelope frequencies.  However, to save time, it assumes that
 * the envelope frequencies are a subset of the source eigenverbs frequencies,
 * so that no interpolation is required.
 *
 * There are no public methods in this class.  It just acts as a set of service
 * routines for the rvbenv_collection class.
 *
 * @xref S. Reilly, D. Thibaudeau, T. Burns, "Fast computation of
 * reverberation using Gaussian beam reflections," Report to NAWCTSD,
 * October 20, 2014.
 */
class USML_DECLSPEC rvbenv_model {
    friend class rvbenv_collection;

   private:
    /// Alias for shared const reference to envelope model.
    typedef std::shared_ptr<const rvbenv_model> csptr;

    /**
     * Initialize model from arguments passed to rvbenv_collection.
     *
     * @param envelope_freq Frequencies at which envelopes are computed.
     * @param travel_time   Times at which the sensor_pair's
     *                      reverberation envelopes are computed (sec).
     * @param threshold     Minimum intensity level for valid reverberation
     *                      contributions (linear units).
     */
    rvbenv_model(const seq_vector::csptr& envelope_freq,
                 const seq_vector::csptr& travel_time);

    /**
     * Computes intensity for a single combination of source and receiver
     * eigenverbs. It assumes that the calling routine has computed the
     * scattering coefficient; which saves this class from having to know
     * anything about the ocean.
     *
     * @param src_verb	Eigenverb contribution from the source
     *                  at the original source frequencies.
     * @param rcv_verb  Eigenverb contribution from the receiver
     *                  interpolated onto the envelope frequencies.
     * @param scatter   Scattering strength coefficient for this
     *                  combination of eigenverbs (ratio).
     * @param xs2       Square of the relative distance from the
     *                  receiver to the target along the direction
     *                  of the receiver's length.
     * @param ys2       Square of the relative distance from the
     *                  receiver to the target along the direction
     *                  of the receiver's width.
     * @return          False if reverberation power below threshold.
     */
    bool add_intensity(const biverb_model::csptr& verb,
                           const vector<double>& scatter, double xs2,
                           double ys2);

    /**
     * Reverberation intensity at each point the time series.
     * Each row represents a specific envelope frequency.
     * Each column represents a specific travel time.
     * Passing this back as a non-const reference allows it to be accessed
     * by a matrix_row<> proxy in the calling program.
     */
    matrix<double>& intensity() { return _intensity; }

    /**
     * Compute the total power and duration of the overlap between
     * two eigenverbs. Implements the analytic solution for power of
     * the bistatic reverberation contribution from eqn. (28) ans (29)
     * in the paper.  Computes the duration from eqn. (45) and (33).
     *
     * @param src_verb		Eigenverb contribution from the source,
     *                      at the original source frequencies.
     * @param rcv_verb      Eigenverb contribution from the receiver,
     *                      interpolated onto the envelope frequencies.
     * @param scatter       Scattering strength coefficient for this
     *                      combination of eigenverbs,
     *                      as a function of envelope frequency (ratio).
     * @param xs2           Square of the relative distance from the
     *                      receiver to the target along the direction
     *                      of the receiver's length.
     * @param ys2           Square of the relative distance from the
     *                      receiver to the target along the direction
     *                      of the receiver's width.
     * @return              False if power below threshold.
     */
    bool compute_overlap(const eigenverb_model::csptr& src_verb,
                         const eigenverb_model::csptr& rcv_verb,
                         const vector<double>& scatter, double xs2, double ys2);

    /**
     * Computes Gaussian time series contribution given delay, duration, and
     * total power.  Implements equation (6) from the paper.  Replaces the
     * values previously held by the _intensity member variable.
     *
     * In an effort to speed up the calculation of the Gaussian, this
     * routine uses uBLAS vector and matrix proxies to only compute the
     * portion of the time series within +/- five (5) times the duration
     * of each pulse.
     *
     * @param src_verb_time		One way travel time for source
     * eigenverb.
     * @param rcv_verb_time     One way travel time for receiver eigenverb.
     */
    void compute_time_series(double src_verb_time, double rcv_verb_time);

    /**
     * Frequencies at which the source and receiver eigenverbs overlap (Hz).
     * Frequencies at which envelope will be computed.
     */
    seq_vector::csptr _envelope_freq;

    /**
     * Times at which the sensor_pair's reverberation envelopes
     * are computed (sec).  These times are not required to be evenly spaced.
     * In the compute_time_series(), the find() method is used on this
     * variable to search for the window in which to deposit the
     * Gaussian contribution from each eigenverb overlap.
     */
    seq_vector::csptr _travel_time;

    /**
     * Minimum intensity level for valid reverberation contributions
     * (linear units).
     */
    const double _threshold{0.0};

    /**
     * Workspace for storing a single eigenverb overlap contribution
     * as a function of two way travel time. Used to build the intensity
     * for an individual frequency in the compute_time_serie() method.
     * Building this as a member variable allows the allocation of memory
     * to be re-used across eigenverb pairs.
     */
    vector<double> _level;

    /**
     * Workspace for storing total power of eigenverb overlap,
     * as a function of envelope frequency (linear units).
     * Used to pass the results of the compute_overlap() method
     * to the compute_time_serie() method.
     * Building this as a member variable allows the allocation of memory
     * to be re-used across eigenverb pairs.
     */
    vector<double> _power;

    /**
     * Workspace for storing duration result of eigenverb overlap (sec).
     * Used to pass the results of the compute_overlap() method
     * to the compute_time_serie() method.
     * Building this as a member variable allows the allocation of memory
     * to be re-used across eigenverb pairs.
     */
    double _duration{0.0};

    /**
     * Computed reverberation intensity at each point the time series.
     * Each row represents a specific envelope frequency.
     * Each column represents a specific travel time.
     * Used to pass results back to the caller.
     * Building this as a member variable allows the allocation of memory
     * to be re-used across eigenverb pairs.
     */
    matrix<double> _intensity;
};

/// @}
}  // namespace rvbenv
}  // end of namespace usml
