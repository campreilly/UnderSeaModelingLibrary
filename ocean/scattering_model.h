/**
 * @file scattering_model.h
 */

#ifndef USML_OCEAN_SCATTERING_MODEL_H
#define USML_OCEAN_SCATTERING_MODEL_H

#include <usml/ublas/ublas.h>
#include <usml/types/types.h>

namespace usml {
namespace ocean {

using namespace usml::ublas ;
using namespace usml::types ;

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
 * Volume reverberation is modeled by integrating the interface scattering
 * strength over the layer thickness. These models compute their results
 * as a function of frequency to support broadband acoustics.
 */
class USML_DECLSPEC scattering_model {

    public:

        /**
         * Computes the broadband scattering strength for a single location.
         *
         * @param location      Location at which to compute attenuation.
         * @param frequencies   Frequencies over which to compute loss. (Hz)
         * @param de_incident   Depression incident angle (radians).
         * @param de_scattered  Depression scattered angle (radians).
         * @param az_incident   Azimuthal incident angle (radians).
         * @param az_scattered  Azimuthal scattered angle (radians).
         * @param amplitude     Change in ray strength in dB (output).
         */
        virtual void scattering( const wposition1& location,
            const seq_vector& frequencies, double de_incident, double de_scattered,
            double az_incident, double az_scattered, vector<double>* amplitude ) = 0 ;

        /**
         * Virtual destructor
         */
        virtual ~scattering_model() {}

};

/// @}
}   // end namespace ocean
}   // end namespace usml

#endif
