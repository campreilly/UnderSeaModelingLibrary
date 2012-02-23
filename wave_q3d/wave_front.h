/** 
 * @file wave_front.h
 * Wavefront characteristics at a specific point in time.
 */
#ifndef USML_WAVEQ3D_WAVE_FRONT_H
#define USML_WAVEQ3D_WAVE_FRONT_H

#include <usml/ocean/ocean.h>

namespace usml {
namespace wave_q3d {

using namespace usml::ocean ;

/// @ingroup wave_q3d
/// @{

/**
 * Wavefront characteristics at a specific point in time. The wave_queue
 * class maintains a circular queue of wave_front objects.  On each 
 * iteration, the wave_front objects computes the derivative terms needed 
 * by the wave_queue's Adams-Bashforth propagator.
 *
 * \f[
 *      \frac{d\rho}{dt} = c^2 \xi_\rho
 * \f]\f[
 *      \frac{d\theta}{dt} = \frac{ c^2 \xi_\theta }{ \rho }
 * \f]\f[
 *      \frac{d\phi}{dt} = \frac{ c^2 \xi_\phi }{ \rho sin(\theta) }
 * \f]\f[
 *      \frac{d\xi_\rho}{dt} = -\frac{1}{c}\frac{dc}{d\rho} +
 *          \frac{c^2}{\rho}\left( \xi^2_\theta + \xi^2_\phi \right)
 * \f]\f[
 *      \frac{d\xi_\theta}{dt} = -\frac{1}{c\rho}\frac{dc}{d\theta} -
 *          \frac{c^2}{\rho} 
 *          \left( \xi_\rho \xi_\theta - \xi^2_\phi cot(\theta) \right)
 * \f]\f[
 *      \frac{d\xi_\phi}{dt} = -\frac{1}{c\rho sin(\theta)}\frac{dc}{d\phi} -
 *          \frac{c^2}{\rho} \xi_\phi
 *          \left( \xi_\rho + \xi_\theta cot(\theta) \right)
 * \f]
 * 
 * where:
 *      - \f$ \rho, \theta, \phi \f$ = position in spherical 
 *        polar coordinates
 *      - \f$ \xi_\rho, \xi_\theta, \xi_\phi \f$  = normalized direction 
 *        in spherical earth coordinates = direction divided 
 *        by speed of sound
 *      - \f$ \frac{d}{dt}_n\f$ = time derivatives
 *      - \f$ c \f$ = speed of sound at this position
 *      - \f$ \frac{dc}{d\rho}, \frac{dc}{d\theta},\frac{dc}{d\phi} \f$ = 
 *        sound speed gradient
 *
 * Note that these derivatives can be computed without any knowledge of the
 * next or previous wavefront.  Any parts of the calculation that require
 * knowledge of next or previous wavefronts are implemented in the
 * wave_queue class.
 *
 * In this implementation, many of the intermediate terms are cached 
 * as private data members in the wave_front object to reduce the 
 * number of times that common terms need to be re-allocated in memory.
 *
 * @xref S.M. Reilly, G. Potty, Sonar Propagation Modeling using Hybrid
 * Gaussian Beams in Spherical/Time Coordinates, January 2012.
 */
class USML_DECLSPEC wave_front {

public:

    /** 
     * Location of each point on the wavefront in spherical earth coordinates.
     * Updated by the propagator each time the wavefront is iterated.
     */
    wposition position ;

    /** 
     * First derivative of position with respect to time.
     * Used by the Adams-Bashforth algorithm to compute the next position.
     */
    wposition pos_gradient ;

    /** 
     * Normalized propagation direction of each point on the wavefront 
     * in spherical earth coordinates.  Equal to the true propagation
     * direction divided by the speed of sound.  Also equal to the
     * wavenumber vector divided by the angular frequency of the sound.
     */
    wvector ndirection ;

    /** 
     * First derivative of normalized direction with respect to time.
     * Used by the Adams-Bashforth algorithm to compute the next direction.
     */
    wvector ndir_gradient ;

    /** 
     * Speed of sound at each point on the wavefront.
     */
    matrix<double> sound_speed ;

    /** 
     * Sound speed gradient at each point on the wavefront.
     */
    wvector sound_gradient ;

    /** 
     * Non-spreading component of propagation loss in dB.
     * Stores the cumulative result of interface reflection losses
     * and losses that result from the attenuation of sound in sea water.
     */
    matrix< vector< double > > attenuation ;

    /** 
     * Non-spreading component of phase change in radians.
     * Stores the cumulative result of the phase changes from
     * interface reflections and caustics.
     */
    matrix< vector< double > > phase ;

    /**
     * Distance from old location to this location.
     * Used by wave_front::compute_profile() to compute the
     * attenuation of sound in sea water for each position
     * change in the wavefront.
     */
    matrix<double> distance ;
    
    /**
     * Cumulative # of surface reflections encountered at this point in time.
     */
    matrix<int> surface ;
    
    /**
     * Cumulative # of bottom reflections encountered at this point in time.
     */
    matrix<int> bottom ;
    
    /**
     * Cumulative # of caustics encountered at this point in time.
     * A caustic is defined as a place on the wavefront where a ray is tangent
     * to the boundary of a shadow zone. The wave_queue::detect_caustic()
     * function updates this field whenever the closest ray to a edge in the
     * wavefront crosses one of its neighbors without changing the number of
     * bounces. Note that this model also subtracts a PI/2 phase shift from the
     * next ray if caustic detected.
     */
    matrix<int> caustic ;

    /**
     * Mark places where the wavefront changes up/down direction.
     */
    matrix<bool> on_fold ;

    /**
     * Mark places where the wavefront changes the surface, bottom, or caustics.
     */
    matrix<bool> on_edge ;

    /**
     * Position of each eigenray target.
     * Reference to data managed by proploss class.
     * Eigenrays are not computed if this reference is NULL.
     */
    const wposition* targets ;
    
    /** 
     * Distance squared from each target to each point on the wavefront.
     * Not used if targets attribute is NULL.
     */
    matrix< matrix<double> > distance2 ;

private:

    /**
     * Environmental parameters.
     * Reference to data managed by wave_queue class.
     */
    ocean_model& _ocean ;

    /**
     * Frequencies over which to compute propagation effects (Hz).
     * Reference to data managed by wave_queue class.
     */
    const seq_vector* _frequencies ;

    /** 
     * Sound speed gradient divided by sound speed (cached intermediate term).
     */
    wvector _dc_c ;

    /** 
     * Square of the speed of sound divided by rho (cached intermediate term).
     */
    matrix<double> _c2_r ;

    /** 
     * Sine of colatitude (cached intermediate term).
     */
    matrix<double> _sin_theta ;
    
    /** 
     * Cotangent of colatitude (cached intermediate term).
     */
    matrix<double> _cot_theta ;

    /**
     * Sin of colatitude for targets (cached intermediate term).
     * Reference to data managed by proploss class.
     * Not used if eigenrays are not being computed.
     */
    const matrix<double>* _target_sin_theta ;

    //**************************************************
    // methods
    
public:

    /**
     * Create workspace for all properties.  Most of the real work of
     * initialization is done after construction so that the previous,
     * current, and next elements can each be initialized differently.
     *
     * @param  ocean        Environmental parameters.
     * @param  freq         Frequencies over which to compute loss (Hz).
     * @param  num_de       Number of D/E angles in the ray fan.
     * @param  num_az       Number of AZ angles in the ray fan.
     * @param  targets      Position of each eigenray target. Eigenrays are not
     *                      computed if this reference is NULL.
     * @param  sin_theta    Reference to sin(theta) for each target.
     *                      Used to speed up compute_target_distance() calc.
     *                      Not used if eigenrays are not being computed.
     */
    wave_front( 
        ocean_model& ocean,
        const seq_vector* freq,
        unsigned num_de, unsigned num_az,
        const wposition* targets = NULL,
        const matrix<double>* sin_theta = NULL
        ) ;

    /**
     * Number of D/E angles in the ray fan.
     */
    inline unsigned num_de() const {
        return position.size1() ;
    }

    /**
     * Number of AZ angles in the ray fan.
     */
    inline unsigned num_az() const {
        return position.size2() ;
    }
    
    /**
     * Initialize position and direction components of the wavefront.
     * Computes normalized directions from depression/elevation 
     * and azimuthal angles.  Each row in the output corresponds
     * to a new depression/elevation angle and and each column 
     * represents a new azimuth. Used during wave_queue class initialization.
     *
     * @param  pos          Initial location in spherical earth coordinates.
     * @param  de           Initial depression/elevation angles at the 
     *                      source location (degrees, positive is up).
     * @param  az           Initial azimuthal angles at the source
     *                      location (degrees, clockwise from true north).
     */
    void init_wave( 
        const wposition1& pos, const seq_vector& de, const seq_vector& az ) ;

    /**
     * Update wave element properties based on the current position 
     * and direction vectors. For each point on the wavefront, it computes 
     * ocean profile parameters, Adams-Bashforth derivatives, and the 
     * distance to each eigenray target.
     */
    void update() ;

    /**
     * Find all folds in the ray fan.  Sets on_fold(de,az) to true if
     * neighboring D/E points change radial direction.
     */
    void find_folds() ;

    /**
     * Find all edges in the ray fan.  Sets on_edge(de,az) to true if
     * it is on the edge of the ray fan or one of its neighbors has
     * a different surface, bottom, or caustic count.
     */
    void find_edges() ;

private:

    /**
     * Compute a fast an approximation of the distance squared from each
     * target to each point on the wavefront.  The speed-up process uses
     * the fact that the haversine distance formula can be replace
     * sin(x/2)^2 with (x/2)^2 when the latitude and longitude differences
     * between points is small.
     * <pre>
     *      distance^2 = r1*r1 + r2*r2 - 2*r1*r2
     *          * { 1-2*( sin^2[(t1-t2)/2] + sin(t1)sin(t2)sin^2[(p1-p2)/2] ) }
     *
     *      distance^2 = r1*r1 + r2*r2 - 2*r1*r2
     *          * { 1-2*( [(t1-t2)/2]^2 + sin(t1)sin(t2)[(p1-p2)/2]^2 ) }
     * </pre>
     * It also uses the fact that sin(x) is precomputed for each target and
     * each point of the wavefront in an eariler step of the update() function.
     * This approach allows us to approximation distances in spherical
     * coordinates without the use of any transindental function.
     */
    void compute_target_distance() ;
                    
    /**
     * Compute extracts the sound_speed, sound_gradient, and attenuation
     * elements of the ocean profile.  It also clears the phase of the
     * wavefront. Later, the reflection_model will incorporate
     * reflection effects and wave_queue::step() will convert them into
     * an accumulated attenuation and phase.
     */
    void compute_profile() ;
                    
};

/// @}
}  // end of namespace wave_q3d
}  // end of namespace usml

#endif
