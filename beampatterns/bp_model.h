/**
 * @file bp_model.h
 * Base class used for building beam pattern models.
 */
#pragma once

#include <usml/types/bvector.h>
#include <usml/types/orientation.h>
#include <usml/types/seq_vector.h>
#include <usml/usml_config.h>

#include <boost/numeric/ublas/vector.hpp>
#include <memory>

namespace usml {
namespace beampatterns {

using namespace usml::types;

/// @ingroup beampatterns
/// @{

/**
 * A "beam pattern" computes the change of intensity for a signal arriving
 * from a given arrival angle as a function of frequency, steering angle, and
 * the local speed of sound. Beam patterns are provided in linear units
 * with a range from 0.0 to 1.0.  All of the beam patterns defined in this
 * module are immutable to support thread safety without locking.
 *
 * We define the directivity gain (DG) for each beam pattern in linear units
 * such that, when multiplied by the ambient noise intensity, it yields
 * the noise intensity perceived by the sensor.
 * \f[
 *      DG =\frac{1}{4 \pi} \int \int b(\theta,\phi) \cos{\theta} d\theta d\phi
 * \f]
 *
 * @xref R.J. Urick, Principles of Underwater Sound, 3rd Edition,
 * (1983), p. 42.
 */
class USML_DECLSPEC bp_model {
   public:
    /// Alias for shared const reference to bp_model.
    typedef std::shared_ptr<const bp_model> csptr;

    /**
     * Virtual destructor.
     */
    virtual ~bp_model() {}

    /**
     * A "beam pattern" computes the change of intensity for a signal arriving
     * from a given arrival angle as a function of frequency, steering angle,
     * and the local speed of sound. Beam patterns are provided in linear units
     * with a range from 0.0 to 1.0.
     *
     * @param arrival       Arrival vector relative to body (out from array).
     * @param frequencies   List of frequencies to compute beam level for.
     * @param level         Beam level output for each frequency (linear units)
     * @param steering      Steering vector relative to body.
     * @param sound_speed   Speed of sound in water (m/s).
     */
    virtual void beam_level(const bvector& arrival,
                            const seq_vector::csptr& frequencies,
                            vector<double>* level,
                            const bvector& steering = bvector(1.0, 0.0, 0.0),
                            double sound_speed = 1500.0) const = 0;

    /**
     * Computes the beam level gain for an arrival vector in the body
     * coordinates of an array which has been rotated by 'orient'.
     *
     * @param arrival       Arrival vector relative to body (out from array).
     * @param orient        Orientation of the beam pattern.
     * @param frequencies   List of frequencies to compute beam level for.
     * @param level         Beam level output for each frequency (linear units).
     * @param steering      Steering vector relative to body.
     * @param sound_speed   Speed of sound in water (m/s).
     */
    void beam_level(const bvector& arrival, const orientation& orient,
                    const seq_vector::csptr& frequencies, vector<double>* level,
                    const bvector& steering = bvector(1.0, 0.0, 0.0),
                    double sound_speed = 1500.0) const;

    /**
     * Compute the directivity gain for this beam pattern.
     * The default behavior integrates beam level over a grid of DE and AZ
     * values at 1 degree spacing.
     *
     * @param frequencies   List of frequencies.
     * @param level         Directivity gain for these frequency (output).
     * @param steering      Steering vector relative to body.
     * @param sound_speed   Speed of sound in water (m/s).
     */
    virtual void directivity(const seq_vector::csptr& frequencies,
                             vector<double>* level,
                             const bvector& steering = bvector(1.0, 0.0, 0.0),
                             double sound_speed = 1500.0) const;
};

/// @}
}  // namespace beampatterns
}  // namespace usml
