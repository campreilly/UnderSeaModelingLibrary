/**
 * @file scattering_model.h
 * Generic interface for reverberation scattering strength model.
 */
#pragma once

#include <usml/types/types.h>
#include <usml/ublas/ublas.h>

namespace usml {
namespace ocean {

using namespace usml::ublas;
using namespace usml::types;

using boost::numeric::ublas::vector;

/// @ingroup boundaries
/// @{

/**
 * A "reverberation scattering strength model" computes the changes in
 * amplitude that result from the non-specular scattering of a ray
 * collision with an interface. The directions of the incoming and
 * outgoing rays are specified in terms of bistatic depression/elevation (D/E)
 * and azimuthal angles (AZ) at the scattering patch. Note that
 * depression/elevation (D/E) is the negative of grazing angle.
 * These models compute their results
 * as a function of frequency to support broadband acoustics.
 *
 * When used as part of a "boundary_model", the reverberation scattering
 * strength coefficient is the intensity ratio per unit area.
 * \f[
 *   \frac{I_{scat}}{I_{inc}} = \sigma_A \: \delta A
 * \f]
 * where
 *         \f$ I_{inc} \f$ = scattering strength coefficient,
 *         \f$ I_{scat} \f$ = scattering strength coefficient,
 *         \f$ \delta A \f$ = ensonified area,
 *         \f$ \sigma_A \f$ = interface scattering strength coefficient.
 *
 * When used as part of a "volume_model", the reverberation scattering
 * strength coefficient is the intensity ratio per unit volume.
 * \f[
 *   \frac{I_{scat}}{I_{inc}} = \sigma_V \: \delta V
 * \f]
 * where
 *         \f$ \delta V \f$ = ensonified volume,
 *         \f$ \sigma_V \f$ = volume scattering strength coefficient.
 *
 * The integrated volume scattering strength per unit area is given by
 * \f[
 *   \sigma_A = h \sigma_V
 * \f]
 * where
 *         \f$ h \f$ = thickness of the volume scattering layer.
 */
class USML_DECLSPEC scattering_model {
   public:
    /// Shared pointer to constant version of this class.
    typedef std::shared_ptr<const scattering_model> csptr;

    /**
     * Computes the broadband scattering strength for a single location.
     *
     * @param location      Location at which to compute attenuation.
     * @param frequencies   Frequencies over which to compute loss. (Hz)
     * @param de_incident   Depression incident angle (radians).
     * @param de_scattered  Depression scattered angle (radians).
     * @param az_incident   Azimuthal incident angle (radians).
     * @param az_scattered  Azimuthal scattered angle (radians).
     * @param amplitude     Reverberation scattering strength ratio (output).
     */
    virtual void scattering(const wposition1& location,
                            seq_vector::csptr frequencies, double de_incident,
                            double de_scattered, double az_incident,
                            double az_scattered,
                            vector<double>* amplitude) const = 0;

    /**
     * Computes the broadband scattering strength for a collection of
     * scattering angles from a common incoming ray. Each scattering
     * has its own location, de_scattered, and az_scattered.
     * The result is a broadband reverberation scattering strength for
     * each scattering.
     *
     * @param location      Location at which to compute attenuation.
     * @param frequencies   Frequencies over which to compute loss. (Hz)
     * @param de_incident   Depression incident angle (radians).
     * @param de_scattered  Depression scattered angle (radians).
     * @param az_incident   Azimuthal incident angle (radians).
     * @param az_scattered  Azimuthal scattered angle (radians).
     * @param amplitude     Reverberation scattering strength ratio (output).
     */
    virtual void scattering(const wposition& location,
                            seq_vector::csptr frequencies, double de_incident,
                            matrix<double> de_scattered, double az_incident,
                            matrix<double> az_scattered,
                            matrix<vector<double> >* amplitude) const = 0;

    /**
     * Virtual destructor
     */
    virtual ~scattering_model() {}
};

/// @}
}  // end namespace ocean
}  // end namespace usml
