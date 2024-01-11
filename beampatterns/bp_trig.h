/**
 * @file bp_trig.h
 * Models frequency independent sine and cosine beampatterns.
 */
#pragma once

#include <usml/beampatterns/bp_model.h>
#include <usml/types/bvector.h>
#include <usml/types/seq_vector.h>
#include <usml/usml_config.h>

#include <boost/numeric/ublas/vector.hpp>
#include <cmath>

namespace usml {
namespace beampatterns {

/// @ingroup beampatterns
/// @{

/// Used to determine which beam-pattern type to implement
enum class bp_trig_type {
    cosine = 0,  ///< MRA to the front
    sine = 1     ///< MRA to the right
};

/**
 * Models frequency independent sonobuoy sine and cosine beampatterns.
 * The steering is fixed relative to the reference axis of the sonobuoy,
 * but the reference axis can be changed by rotating the orientation of
 * the buoy.
 * \f[
 *      B(\vec{u}) = K_n + (1-K_n) \vec{u} \cdot \vec{u}_{ref}
 * \f]
 * where
 * - \f$ \vec{u} \f$   = wavefront direction (out from array),
 * - \f$ \vec{u}_{ref} \f$ = array reference axis (forward
 *   for cosine and right for sine), and
 * - \f$ K_n \f$ = the desired null depth.
 *
 * The directivity gain of either beam pattern has an analytic form.
 * \f[
 *      DG = K_n + (1-K_n) / 3
 * \f]
 */
class USML_DECLSPEC bp_trig : public bp_model {
   public:
    /**
     * Constructs sine or cosine beampattern.
     *
     * @param type		    COSINE or SINE
     * @param null		    Desired null depth (dB).
     */
    bp_trig(bp_trig_type type = bp_trig_type::cosine, double null = -300.0)
        : _type(type), _null(pow(10.0, -abs(null) / 20.0)) {
        _gain = 1.0 - _null;
        _directivity = _null + _gain / 3.0;
    }

    void beam_level(const bvector& arrival, const seq_vector::csptr& frequencies,
                    vector<double>* level,
                    const bvector& steering = bvector(1.0, 0.0, 0.0),
                    double sound_speed = 1500.0) const override;

    void directivity(const seq_vector::csptr& frequencies, vector<double>* level,
                     const bvector& steering = bvector(1.0, 0.0, 0.0),
                     double sound_speed = 1500.0) const override;

   private:
    /// sine or cosine
    const bp_trig_type _type;

    /// Minimum loss value in a null zone (linear, intensity)
    const double _null;

    /// Gain factor of the array, 1.0 - null (linear, intensity)
    double _gain;

    /// Array gain against noise.
    double _directivity;
};

/// @}
}  // namespace beampatterns
}  // namespace usml
