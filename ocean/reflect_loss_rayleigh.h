/**
 * @file reflect_loss_rayleigh.h
 * Models plane wave reflection from a flat fluid-solid interface.
 * Includes LaTEX documentation of formula for processing by Doxygen.
 */

#ifndef USML_OCEAN_REFLECT_LOSS_RAYLEIGH_H
#define USML_OCEAN_REFLECT_LOSS_RAYLEIGH_H

#include <usml/ocean/reflect_loss_model.h>

namespace usml {
namespace ocean {

using boost::numeric::ublas::vector;

/// @ingroup boundaries
/// @{

/**
 * Models plane wave reflection loss from a flat fluid-solid interface.
 * Includes the effects of both compression and shear waves in the bottom.
 * Note that the Rayleigh model is frequency independent because
 * all of the frequency terms cancel out.
 *
 * The effect of attenuation is incorporated into the model as
 * a complex component of the sound speed:
 * \f[
 *          c_w = c_{rw}
 * \f]\f[
 *          c_b = c_{rb}-i \frac{ \alpha_b c_{rb}^2 }{ \omega }
 * \f]\f[
 *          \alpha_b = \frac{ \alpha_{\lambda b} }{ \lambda_b 20 \log{e} }
 * \f]
 * where:
 *   - \f$ c_n        \f$ = complex sound speed in medium "n" (m/s)
 *   - \f$ c_{rn}     \f$ = real component of sound speed in medium "n" (m/s)
 *   - \f$ \alpha_{\lambda b} \f$ = attenuation coefficient in (dB/wavelength)
 *   - \f$ \lambda_b  \f$ = wavelength in bottom = \f$ c_{rb} / f \f$
 *   - \f$ \alpha_b   \f$ = attenuation coefficient in bottom (nepers/meter)
 *
 * The effect of absorption on the in-water sound speed is
 * assumed to be negligible.
 *
 * The complex reflection coefficient is modeled as a combination of
 * the acoustic impedances for compressional and shear waves.
 * The effect of shear on the in-water impedance is assumed to be negligible.
 * \f[
 *          Z_{pn} = \frac{ \rho_n c_n }{ cos(\theta_{pn}) }
 * \f]\f[
 *          Z_{sb} = \frac{ \rho_b c_{sb} }{ cos(\theta_{sb}) }
 * \f]\f[
 *          Z_w = Z_{pw}
 * \f]\f[
 *          Z_b = Z_{pb} sin^2(2\theta_{sn}) + Z_{sb} cos^2(2\theta_{sn})
 * \f]\f[
 *          R = \frac{ Z_b - Z_w }{ Z_b + Z_w }
 * \f]
 * where:
 *   - \f$ \rho_n  \f$ = density of medium "n" (kg/m^3)
 *   - \f$ c_{sb}  \f$ = complex shear speed in bottom (m/s)
 *   - \f$ \theta_{pn} \f$ = angle between ray and surface normal
 *          in medium "n" for compression waves (radians)
 *   - \f$ \theta_{sn} \f$ = angle between ray and surface normal
 *          in bottom for shear waves (radians)
 *   - \f$ Z_{pn}  \f$ = compression waves impedance in medium "n"
 *   - \f$ Z_{sn}  \f$ = shear wave impedance in bottom
 *   - \f$ Z_n     \f$ = total acoustic impedance in medium "n"
 *   - \f$ R       \f$ = complex reflection coefficient
 *
 * The angles between the ray and surface normal in each medium is
 * computed using Snell's Law:
 * \f[
 *      \frac{sin(\theta_w)}{c_w} =
 *      \frac{sin(\theta_{pb})}{c_{pb}} =
 *      \frac{sin(\theta_{sb})}{c_{sb}}
 * \f]
 *
 * Note that the sin() and cos() terms in this derivation have been
 * inverted from the reference to take into account the difference
 * between grazing angle and angle to the surface normal.
 *
 * @xref F.B. Jensen, W.A. Kuperman, M.B. Porter, H. Schmidt,
 * "Computational Ocean Acoustics", pp. 35-49.
 */
class USML_DECLSPEC reflect_loss_rayleigh : public reflect_loss_model {

public:

    /** Bottom types supported by table lookup feature. */
    typedef enum {
        CLAY, SILT, SAND, GRAVEL, MORAINE, CHALK, LIMESTONE, BASALT
    } bottom_type_enum ;


    /**
     * Initialize model with a generic bottom type.  Uses an internal
     * lookup table to convert into impedance mis-match factors.
     *
     * @param type          Generic bottom for table lookup of
     *                      impedance mis-match factors.
     */
    reflect_loss_rayleigh( bottom_type_enum type ) ;

    /**
     * Initialize model with a generic bottom type as integer
     * representation.  Uses an internal lookup table to convert
     * into impedance mis-match factors.
     *
     * @param type          Integer representation of generic bottom type.
     */
    reflect_loss_rayleigh( int type ) ;

    /**
     * Initialize model with impedance mis-match factors.  Defined in terms
     * of ratios to match commonly used databases.
     *
     * @param density       Ratio of bottom density to water density
     *                      Water density is assumed to be 1000 kg/m^3.
     * @param speed         Ratio of compressional sound speed in the bottom to
     *                      the sound speed in water. The sound speed in water
     *                      is assumed to be 1500 m/s.
     * @param att_bottom    Compressional wave attenuation in bottom
     *                      (dB/wavelength).  No attenuation if this is zero.
     * @param speed_shear   Ratio of shear wave sound speed in the bottom to
     *                      the sound speed in water.
     * @param att_shear     Shear wave attenuation in bottom (dB/wavelength).
     */
    reflect_loss_rayleigh(
        double density, double speed, double att_bottom=0.0,
        double speed_shear=0.0, double att_shear=0.0 ) ;

    /**
     * Computes the broadband reflection loss and phase change.
     *
     * @param location      Location at which to compute attenuation.
     * @param frequencies   Frequencies over which to compute loss. (Hz)
     * @param angle         Reflection angle relative to the normal (radians).
     * @param amplitude     Change in ray strength in dB (output).
     * @param phase         Change in ray phase in radians (output).
     *                      Phase change not computed if this is NULL.
     */
    virtual void reflect_loss(
        const wposition1& location,
        const seq_vector& frequencies, double angle,
        vector<double>* amplitude, vector<double>* phase=NULL ) ;

private:

    /**
     * Computes the impedance for compression or shear waves with attenuation.
     * Includes the Snell's Law computation of transmitted angle.
     *
     * @param density       Ratio of bottom density to water density
     *                      Water density is assumed to be 1000 kg/m^3.
     * @param speed         Ratio of compressional sound speed in the bottom to
     *                      the sound speed in water. The sound speed in water
     *                      is assumed to be 1500 m/s.
     * @param attenuation   Compressional wave attenuation in bottom
     *                      (dB/wavelength).  No attenuation if this is zero.
     * @param angle         Reflection angle relative to the normal (radians).
     * @param cosA          Returns the cosine of the transmitted angle
     *                      computed using Snell's Law.
     * @param shear         Treat impendance for shear instances as special
     *                      cases.
     */
    complex<double> impedance(
        double density, double speed, double attenuation, double angle,
        complex< double >* cosA, bool shear ) ;

    // data members

    /** Bottom types lookup table. */
    static struct bottom_type_table {
        bottom_type_enum type ;
        double density ;
        double speed ;
        double att_bottom ;
        double speed_shear ;
        double att_shear ;
    } lookup[] ;

    //**************************************************
    // impedance mis-match factors

    /** Density of water (kg/m^3). */
    const double _density_water ;

    /** Speed of sound in water (m/s). */
    const double _speed_water ;

    /** Bottom density (kg/m^3). */
    const double _density_bottom ;

    /** Compressional speed of sound in bottom (m/s). */
    const double _speed_bottom ;

    /** Compressional wave attenuation in bottom (nepers/wavelength). */
    const double _att_bottom ;

    /** Shear speed of sound in bottom (m/s). */
    const double _speed_shear ;

    /** Shear wave attenuation in bottom (nepers/wavelength). */
    const double _att_shear ;

} ;

/// @}
}  // end of namespace ocean
}  // end of namespace usml

#endif
