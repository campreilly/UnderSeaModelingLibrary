/** 
 * @file spreading_hybrid_gaussian.h
 * Spreading loss based on a hybrid Gaussian beam theory.
 */
#ifndef USML_WAVEQ3D_SPREADING_HYBRID_GAUSSIAN_H
#define USML_WAVEQ3D_SPREADING_HYBRID_GAUSSIAN_H

#include <usml/waveq3d/spreading_model.h>

namespace usml {
namespace waveq3d {

using namespace usml::ocean ;

/**
 * @internal
 * Spreading loss based on a hybrid Gaussian beam theory. It is similar
 * to the Gaussian Ray Bundling (GRAB) used by the Weinberg/Keenan model in
 * that the Gaussian profile is defined by the distance between rays 
 * instead of dynamic ray tracing equations.  It also uses the GRAB
 * values for minimum beam width.
 *
 * The intensity at the point of collision is an in-phase summation of 
 * the Gaussian beams that surround the eigenray target.  To create the
 * acoustic field in two dimensions across the wavefront, we assume that 
 * the divergence can be characterized in terms of independent D/E and AZ 
 * terms and that Gaussian beam cross terms are unimportant.
 *
 * The width of each Gaussian beam consists of two components: a frequency 
 * independent cell width and a frequency-dependent evanescent spreading term.
 * The evanescent spreading term is modeled as the distance which a wave 
 * (of a given frequency) could be expected to tunnel into an area 
 * forbidden by classic ray theory.  This distance is equivalent to the
 * \f$ 2 \pi \lambda \f$ minimum width term in the GRAB model.
 * 
 * Creates temporary workspace for many of its frequency dependent terms
 * in the form of member variables so that they don't have to be re-created
 * each time they are used.
 *
 * @xref H. Weinberg, R. E. Keenan, “Gaussian ray bundles for modeling
 * high-frequency propagation loss under shallow-water conditions.”
 * J. Acoust. Soc. Am. 100 (3), 1421-1431, (Sept 1996).
 */
class USML_DECLSPEC spreading_hybrid_gaussian : public spreading_model {

    friend class wave_queue ;
    
  private:

    /** Normalization in depression/elevation direction. */
    vector<double> _norm_de ;

    /** Normalization in azimuthal direction. */
    matrix<double> _norm_az ;

    /** Combination of cell width and spreading. (temp workspace) */
    vector<double> _beam_width ;

    /** Intensity contribution in D/E direction. (temp workspace) */
    vector<double> _intensity_de ;

    /** Intensity contribution in azimuthal direction. (temp workspace) */
    vector<double> _intensity_az ;

    /**
     * Number of wavelengths that each Gaussian beam can be expected
     * spread into neighboring beams.  Equivalent to the
     * \f$ 2 \pi \lambda \f$ minimum width term in the GRAB model.
     */
    static const double SPREADING_WIDTH ; // 2 pi

    /**
     * Minimum percentage that each Gaussian beam can be expected
     * spread into neighboring beams.  When the frequency is very high,
     * the summation across Gaussian beams can have non-physical "ripples"
     * in it.  Follow the GRAB example of 50% overlap.
     */
    static const double OVERLAP ; // 2.0

    /**
     * Limits the extent of the search for Gaussian beam contributions.
     * Iteration stops when new contribution makes less than a 0.01 dB
     * contribution to the overall result.
     */
    static const double THRESHOLD ; // 1.002305238

  protected:

    /**
     * Normalize each wavefront cell by the surface area it takes up
     * one meter from source.
     * \f[
     *      A_{DE}(n) = ( DE_{n+1}-DE_n )
     * \f] \f[
     *      A_{AZ}(n,m) = \frac{ ( sin(DE_{n+1})-sin(DE_n) )(AZ_{m+1}-AZ_m) }
     *                         { DE_{n-1} - DE_n }
     * \f]
     * Note that in this implementation the \f$ \sqrt{ 2 \pi } \f$ term
     * from the gaussian() method is folded into the normalization 
     * coefficients so that it can be computed a single time,
     * during initialization.
     *
     * @param wave          Wavefront object associated with this model.
     */
    spreading_hybrid_gaussian( wave_queue& wave ) ;

    /**
     * Virtual destructor
     */
    virtual ~spreading_hybrid_gaussian() {}

    /**
     * Compute the Gaussian contribution from a single wavefront cell.
     * \f[
     *      \frac{A}{w\sqrt{2\pi}} exp\left( - \frac{d^2}{2w^2} \right)
     * \f]
     * The width of each Gaussian beam consists of two components: a frequency
     * independent cell width and a frequency-dependent evanescent spreading
     * term.  If we assume that these two effects combine like the convolution
     * of two Gaussians, then the square of total width will be the sum
     * of squares of each term.
     * \f[
     *      w^2_total = w^2_spread + w^2_width
     * \f]
     * Note that in this implementation the \f$ \sqrt{ 2 \pi } \f$ term
     * is folded into the normalization calculation so that it can be 
     * computed a single time, during initialization.
     *
     * @param   d           Distance from field point to center of profile.
     * @param   w           Half-width this cell in the wavefront.
     * @param   A           Normalization coefficient.
     *
     * @xref Weisstein, Eric W. "Convolution." From MathWorld--A Wolfram Web
     * Resource. http://mathworld.wolfram.com/Convolution.html
     */
    inline vector<double> gaussian(double d,double w,double A) {
        _beam_width = _spread + OVERLAP * OVERLAP * w * w ; // sum of squares
        return element_div( 
        	exp( (-0.5*d*d) / _beam_width ), 
        	sqrt(_beam_width) ) * A ;
    }

    /**
     * Estimate intensity as the product of Gaussian contributions in the 
     * D/E and AZ directions.  It assumes that the the divergence can be
     * characterized in terms of independent D/E and AZ terms and that
     * Gaussian beam cross terms are unimportant.
     *
     * @param  location     Target location.
     * @param  de           DE index of closest point of approach.
     * @param  az           AZ index of closest point of approach.
     * @param  offset       Offsets in time, DE, and AZ at collision.
     * @param  distance     Offsets in distance units.
     * @return              Intensity of ray at this point.
     */
    virtual const vector<double>& intensity(
        const wposition1& location, unsigned de, unsigned az,
        const vector<double>& offset, const vector<double>& distance ) ;

  private:

    /**
     * Summation of Gaussian beam contributions from all cells in 
     * the D/E direction.  Iteration stops when lowest frequency contribution
     * makes less than a THRESHOLD difference relative to the overall result.
     *
     * @param  de           DE index of contributing cell.
     * @param  az           AZ index of contributing cell.
     * @param  offset       Offsets in time, DE, and AZ at collision.
     * @param  distance     Offsets in distance units.
     * @return              Intensity of ray at this point.
     */
    void intensity_de( unsigned de, unsigned az, 
        const vector<double>& offset, const vector<double>& distance ) ;
        
    /**
     * Summation of Gaussian beam contributions from all cells in 
     * the AZ direction.  Iteration stops when lowest frequency contribution
     * makes less than a THRESHOLD difference relative to the overall result.
     *
     * @param  de           DE index of contributing cell.
     * @param  az           AZ index of contributing cell.
     * @param  offset       Offsets in time, DE, and AZ at collision.
     * @param  distance     Offsets in distance units.
     * @return              Intensity of ray at this point.
     */
    void intensity_az( unsigned de, unsigned az, 
        const vector<double>& offset, const vector<double>& distance ) ;
        
    /**
     * Interpolate the half-width of a cell in the D/E direction.
     * At each AZ, compute the distance between the D/E corner
     * and the D/E+1 corner.  Use the AZ offset to linearly interpolate 
     * between these sides.  Then repeat this process with the next
     * (or previous) wavefront and use the time offset to linearly 
     * interpolate between times.
     *
     * @param   de          DE index of contributing cell.
     * @param   az          AZ index of contributing cell.
     * @param   offset      Offsets in time, DE, and AZ at collision.
     * @return              Half-width of cell in the DE direction.
     */
    double width_de( unsigned de, unsigned az, const vector<double>& offset ) ;
    
    /**
     * Interpolate the half-width of a cell in the AZ direction.
     * At each DE, compute the distance between the AZ corner
     * and the AZ+1 corner.  Use the D/E offset to linearly interpolate 
     * between these sides.  Then repeat this process with the next
     * (or previous) wavefront and use the time offset to linearly 
     * interpolate between times.
     *
     * @param   de          DE index of contributing cell.
     * @param   az          AZ index of contributing cell.
     * @param   offset      Offsets in time, DE, and AZ at collision.
     * @return              Half-width of cell in the AZ direction.
     */
    double width_az( unsigned de, unsigned az, const vector<double>& offset ) ;

} ;

}  // end of namespace waveq3d
}  // end of namespace usml

#endif
