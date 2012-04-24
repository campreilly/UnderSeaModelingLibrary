/** 
 * @file reflection_model.h
 * Reflection model components of wave_queue object type.
 */
#ifndef USML_WAVEQ3D_REFLECTION_MODEL_H
#define USML_WAVEQ3D_REFLECTION_MODEL_H

#include <usml/waveq3d/wave_queue.h>

namespace usml {
namespace waveq3d {

using namespace usml::ocean ;
class wave_queue ;      // forward reference for friend declaration

/**
 * @internal
 * Reflection model components of wave_queue object type.
 * These routines have been broken out into their own class just to
 * simplify maintenance of the reflection model components separately
 * from the core of the propagation engine.
 *
 * The  wave_queue::detect_reflections_surface() and
 * wave_queue::detect_reflections_bottom() routines detect
 * when a collision has occurred. Then, this class is used to:
 *
 * - compute a refined location and grazing angle of the collision,
 * - reflect the current wavefront into a new direction, and
 * - re-initialize the wavefront queue for the reflected ray such that
 *   it appears to be coming from an image source on the other side
 *   of the interface.
 *
 * The accuracy limits in this part of the model cause slight fluctuations in
 * the direction of the reflected rays.  If a very finely gridded fan is
 * used, these fluctuation will manifest themselves as gaps between each
 * groups of rays that encountered the interface at a different time steps.
 * Since the divergence of the wavefront is estimated using the spreading
 * between rays, this causes a discontinuity in the propagation loss.
 * When the ray spacing is coarser, these gaps are washed out in the
 * other approximations between rays.  This effect can also be minimized
 * by decreasing the time step.
 *
 * @todo Calculate eigenray amplitude and phase for reverberation callback.
 * Just passing bogus values currently.
 *
 * @xref S.M. Reilly, G. Potty, Sonar Propagation Modeling using Hybrid
 * Gaussian Beams in Spherical/Time Coordinates, January 2012.
 */
class USML_DECLSPEC reflection_model {

    friend class wave_queue ;
    
  private:

    /** Wavefront object associated with this model. */
    wave_queue& _wave ;

    /** Callback model for bottom reverberation */
    reverb_model* _bottom_reverb ;

    /** Callback model for surface reverberation */
    reverb_model* _surface_reverb ;

    /**
     * If the water is too shallow, bottom_reflection() uses a horizontal 
     * normal to simulate reflection from "dry land".  Without this, the 
     * propagation could wander into a region where the ocean bottom was 
     * above the surface and all propagation elements evaluated to NaN.  
     * This approximation has very little practical effect because the rays 
     * are already very weak, due to multiple bottom interactions, 
     * by the time they reach the beach.
     *
     * It is automatically set to a value that is 300 times the time
     * step of the wavefront.  This value 1/5 the length of a typical
     * time step (1500*dt).
     */
    const double TOO_SHALLOW ;

    /**
     * Hide default constructor to prohibit use by non-friends.
     */
    reflection_model( wave_queue& wave ) 
    	: _wave( wave ), _bottom_reverb(0), _surface_reverb(0),
        TOO_SHALLOW( 300.0 * wave._time_step )
    	{}

    /**
     * Reflect a single acoustic ray from the ocean bottom.  
     * Computes boundary reflection loss and re-initializes the direction of 
     * the ray.  Adds reflection attenuation and phase to existing values.
     *
     * The distance (in time) from the "current" wavefront to the 
     * boundary collision is given by:
     * \f[
     *      \Delta\tau_{collision} = 
     *             h \frac{ \hat{r} \bullet \hat{n} }
     *             { \frac{d\vec{r}}{d\tau} \bullet \hat{n} }
     * \f]
     * where:
     *      - h = height above bottom at "current" wavefront
     *      - \f$ \vec{r} \f$    = position in spherical earth coords
     *      - \f$ \hat{r} \f$    = position normal in spherical earth coords
     *      - \f$ \hat{n} \f$    = surface normal
     *      - \f$ \Delta\tau_{collision} \f$ = time step needed for collision
     *
     * The direction of the reflected ray is given by the vector equation:
     * \f[
     *      \hat{R} = \hat{I} - 2 (\hat{I} \bullet \hat{n}) \hat{n}
     * \f]
     * where:
     *      - \f$ \hat{n} \f$ = surface normal
     *      - \f$ \hat{I} \f$ = incident direction in spherical earth coords
     *      - \f$ \hat{R} \f$ = reflected direction in spherical earth coords
     *
     * This routine exits without producing a reflection if this calculation
     * indicates that a near-miss has occurred. A near-miss is defined as the
     * case where the grazing angle is zero or negative. In a near-miss,
     * the ray is already heading back into the water column without the
     * help of a reflection.
     *
     * @param de                D/E angle index number of reflected ray.
     * @param az                AZ angle index number of reflected ray.
     * @param depth             Depth that ray has penetrated into the bottom.
     * @return                  True for an actual reflection,
     *                          False for a near-miss.
     */
    bool bottom_reflection( unsigned de, unsigned az, double depth ) ;

    /**
     * Reflect a single acoustic ray from the ocean surface. 
     * Computes boundary reflection loss and re-initializes the direction of 
     * the ray.  Adds reflection attenuation and phase to existing values.
     *
     * Because the ocean surface has fixed normal, the generic
     * collision equations (used by the bottom_reflection() method)
     * can be simplified into the form:
     * \f[
     *      \Delta\tau_{collision} = \frac{ h }{ \frac{dr}{d\tau} }
     * \f]\f[
     *      R_{\rho} = - I_{\rho}, R_{\theta} = I_{\theta}, R_{\phi} = I_{\phi}
     * \f]
     * where:
     *      - h = distance from current ray to ocean surface
     *      - r = radial component of position in spherical earth coords
     *      - \f$ \Delta\tau_{collision} \f$ = time step needed for collision
     *      - \f$ \hat{I} \f$ = incident direction in spherical earth coords
     *      - \f$ \hat{R} \f$ = reflected direction in spherical earth coords
     *
     * The direction of the reflected ray is computed by taking the negative
     * of the radial component.
     *
     * This routine exits without producing a reflection if this calculation
     * indicates that a near-miss has occurred. A near-miss is defined as the
     * case where the grazing angle is zero or negative. In a near-miss,
     * the ray is already heading back into the water column without the
     * help of a reflection.
     *
     * @param de            D/E angle index number of reflected ray.
     * @param az            AZ angle index number of reflected ray.
     * @return              True for an actual reflection,
     *                      False for a near-miss.
     */
    bool surface_reflection( unsigned de, unsigned az ) ;

    /**
     * Computes a refined location and direction at the point of collision.
     * Uses a second order Taylor series around the current location to
     * estimate these values.
     *
     * @param de            D/E angle index number.
     * @param az            AZ angle index number.
     * @param dtime         The distance (in time) from the "current"
     *                      wavefront to the boundary collision.
     * @param position      Refined position of the reflection (output).
     * @param ndirection    Normalized direction at the point
     *                      of reflection (output).
     * @param speed         Speed of sound at the point of reflection (output).
     */
    void collision_location(
        unsigned de, unsigned az, double dtime,
        wposition1* position, wvector1* ndirection, double* speed ) const ;

    /**
     * Re-initialize an individual ray after reflection.
     * Uses the position and reflected direction to initialize
     * a temporary 1x1 wavefront. Then, the position and direction
     * of the 1x1 wavefront are copied into the reflected ray.
     *
     * @param de            D/E angle index number of reflected ray.
     * @param az            AZ angle index number of reflected ray.
     * @param dtime         The distance (in time) from the "current"
     *                      wavefront to the boundary collision.
     * @param position      Position of the reflection.
     * @param direction     Direction (un-normalized) after reflection.
     * @param speed         Speed of sound at the point of reflection.
     */
    void reflection_reinit( 
        unsigned de, unsigned az, double dtime, 
        const wposition1& position, const wvector1& direction, double speed ) ;

    /**
     * Copy new wave element data into the destination wavefront.
     * Used by reflection_reinit() to change the direction of one
     * ray in the wavefront.
     *
     * @param element       Destination for new information.
     * @param de            D/E angle index number of reflected ray.
     * @param az            AZ angle index number of reflected ray.
     * @param results       Wave element data with new information.
     */
    static void reflection_copy( 
        wave_front* element, unsigned de, unsigned az, 
        wave_front& results ) ;
} ;

}  // end of namespace waveq3d
}  // end of namespace usml

#endif
