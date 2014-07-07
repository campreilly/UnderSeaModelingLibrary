/**
 * @file spreading_ray.h
 * Spreading loss based on classic ray theory.
 */
#ifndef USML_WAVEQ3D_SPREADING_RAY_H
#define USML_WAVEQ3D_SPREADING_RAY_H

#include <usml/waveq3d/spreading_model.h>

namespace usml {
namespace waveq3d {

using namespace usml::ocean ;

/**
 * @internal
 * Spreading loss based on classic ray theory.  The propagation loss within
 * an area bounded by the rays for [de1,de2] and [az1,az2] is proportional
 * to the change in the ensonified area.  If the initial intensity is unity,
 * then the intensity change at the target is given by:
 *
 * \f[
 *      I_{target} = \frac{ A_target / c_target }{ A_0 / c_0 }
 * \f]
 *
 * where:
 *      - \f$ I_{target}    \f$ = intensity change at target
 *      - \f$ A_{target}    \f$ = ensonified area at target
 *      - \f$ A_0           \f$ = initial ensonified area
 *      - \f$ c_{target}    \f$ = speed of sound at target
 *      - \f$ c_0           \f$ = initial speed of sound
 */
class USML_DECLSPEC spreading_ray : public spreading_model {

    friend class wave_queue ;

  private:

    /**
     * Initial ensonified area for each ray span. Assign the area for each
     * span to the index of the ray that precedes it in D/E and azimuth.
     * Copy the last element in each direction from  the one before it.
     * Divide the initial area by the initial speed of sound so that we
     * don't have to do this each time intensity is calculated.
     */
    matrix<double> _init_area ;

  protected:

    /**
     * Estimate initial ensonfied area between rays at radius of 1 meter.
     *
     * \f[
     *      A_{ab} = \left( cos( \mu_b ) - cos( mu_a) \right)
     *               \left( \eta_b - \eta_a\right)
     * \f]
     *
     * where:
     *      - \f$ \mu_a, \mu_b      \f$ = depression/elevation angles at source
     *      - \f$ \eta_a, \eta_b    \f$ = azimuthal angles at source
     *      - \f$ A_{ab} =          \f$ = initial area of interest between
     *                                    these rays
     *
     * @param wave          Wavefront object associated with this model.
     */
    spreading_ray( wave_queue& wave ) ;

    /**
     * Virtual destructor
     */
    virtual ~spreading_ray() {}

    /**
     * Estimate intensity as the ratio of current area to initial area.
     * Approximates the area as the sum of two triangles that connect
     * the corner points.
     *
     * Extrapolates across edges in the wavefront by keeping a constant
     * level for three (3) extra beam widths.  This compensates for the fact
     * that the detection scheme used by wavefront.on_edges() may leave the
     * closest valid wavefront segment may be up to 1 1/2 segments
     * away from the actual edge.  A failure to properly take this into account
     * will show up as weak eignerays near the surface, bottom, or caustics.
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

    /**
     * Compute the harmonic mean of the distance between the position
     * on the wavefront that is DE+1 and DE-1 from DE. Divides the result
     * by 1/2, as the width of the gaussian cell is half of the distance.
     *
     * @param   de          DE index of contributing cell.
     * @param   az          AZ index of contributing cell.
     * @param   offset      (Not used in this calculation)
     * @return              Half-width of cell in the DE direction.
     */
    virtual double width_de(
        unsigned de, unsigned az, const vector<double>& offset ) ;

    /**
     * Compute the harmonic mean of the distance between the position
     * on the wavefront that is AZ+1 and AZ-1 from AZ. Divides the result
     * by 1/2, as the width of the gaussian cell is half of the distance.
     *
     * @param   de          DE index of contributing cell.
     * @param   az          AZ index of contributing cell.
     * @param   offset      (Not used in this calculation)
     * @return              Half-width of cell in the AZ direction.
     */
    virtual double width_az(
        unsigned de, unsigned az, const vector<double>& offset ) ;

} ;

}  // end of namespace waveq3d
}  // end of namespace usml

#endif
