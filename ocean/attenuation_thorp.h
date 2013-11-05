/** 
 * @file attenuation_thorp.h
 * Models attenuation loss using Thorp's model.
 */

#ifndef USML_OCEAN_ATTENUATION_THORP_H
#define USML_OCEAN_ATTENUATION_THORP_H

#include <usml/ocean/attenuation_model.h>

namespace usml {
namespace ocean {

using boost::numeric::ublas::vector;

/// @ingroup profiles
/// @{

/**
 * Models attenuation loss using the Thorp's model.
 * <pre>
 *      attenuation (dB/km) = 
 *            0.0033 + F2* ( 3.0e-4 + 44.0/(4100.0 + F2) 
 *          + 0.11/(1.0 + F2) );
 *
 * where:
 *          F2  = square of frequency in kilohertz
 * </pre>
 * Valid for: 1000 m, 4 deg C, 35 ppt salinity, pH=8, 100-5000 Hz.
 *
 * @xref F.B. Jensen, W.A. Kuperman, M.B. Porter, H. Schmidt,
 * "Computational Ocean Acoustics", Chapter 1.5.2, pp. 37,38.
 *
 * The effect of pressure on absorption is taken into account with a depth
 * correction term:
 * <pre>
 *      attenuation(d0) = attenuation(d0) 
 *                       ( 1 - 5.88264e-6 d ) / ( 1 - 5.88264e-6 d0 )
 *
 * where:
 *      d               = depth in meters.
 *      d0              = reference depth in meters (1000 m for Thorp).
 *      attenuation(d0) = attenuation at reference depth.
 * </pre>
 *
 * This implementation saves computation time by caching the attenuation
 * coefficients and only re-computing them if the frequency changes.
 *
 * @xref R.J. Urick, Principles of Underwater Sound, 3rd Edition, 
 * (1983), p. 108.
 *
 * @xref R.H. Fisher, "Effect of High Pressure on Sound Absorption
 * and Chemical Equilibrium," J. Acoust. Soc. Am. 30:442 (1973).
 */
class USML_DECLSPEC attenuation_thorp : public attenuation_model {

  public:

    /**
     * Default initializes does nothing.
     */
    attenuation_thorp(){}

    /**
     * Virtual destructor
     */
    virtual ~attenuation_thorp() {}

    /**
     * Computes the broadband absorption loss of sea water.
     *
     * @param location      Location at which to compute attenuation.
     * @param frequencies   Frequencies over which to compute loss. (Hz)
     * @param distance      Distance traveled through the water (meters).
     * @param attenuation   Absorption loss of sea water in dB (output).
     */
    virtual void attenuation( 
        const wposition& location, 
        const seq_vector& frequencies,
        const matrix<double>& distance,
        matrix< vector<double> >* attenuation ) ;
        
} ;

/// @}
}  // end of namespace ocean
}  // end of namespace usml

#endif
