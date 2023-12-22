/**
 * @file wave_queue.h
 * Wavefront propagator for the WaveQ3D model.
 */
#pragma once

#include <netcdfcpp.h>
#include <usml/eigenrays/eigenray_notifier.h>
#include <usml/eigenverbs/eigenverb_notifier.h>
#include <usml/ocean/ocean_model.h>
#include <usml/types/seq_vector.h>
#include <usml/types/wposition.h>
#include <usml/types/wposition1.h>
#include <usml/usml_config.h>
#include <usml/waveq3d/reflection_notifier.h>
#include <usml/waveq3d/wave_front.h>
#include <usml/waveq3d/wave_thresholds.h>

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <cstddef>
#include <memory>

namespace usml {
namespace waveq3d {

using namespace usml::ocean;
using namespace usml::eigenverbs;
using namespace usml::eigenrays;

class reflection_model;
class spreading_model;
class spreading_ray;
class spreading_hybrid_gaussian;

/// @ingroup waveq3d
/// @{

/**
 * Wavefront propagator for the WaveQ3D model. Implemented using a
 * third order Adams-Bashforth algorithm which estimates the new
 * position and direction from the previous three iterations.
 *
 * \f[
 *      \vec{r}_{n+1} = \vec{r}_{n} + \Delta t \left(
 *          \frac{23}{12} \frac{ d \vec{r} }{ dt }_n -
 *          \frac{16}{12} \frac{ d \vec{r} }{ dt }_{n-1} +
 *          \frac{ 5}{12} \frac{ d \vec{r} }{ dt }_{n-2}
 *          \right)
 * \f]\f[
 *      \vec{\xi}_{n+1} = \vec{\xi}_{n} + \Delta t \left(
 *          \frac{23}{12} \frac{ d \vec{\xi} }{ dt }_n -
 *          \frac{16}{12} \frac{ d \vec{\xi} }{ dt }_{n-1} +
 *          \frac{ 5}{12} \frac{ d \vec{\xi} }{ dt }_{n-2}
 *          \right)
 * \f]
 * where:
 *      - \f$ \vec{r}       \f$ = position in spherical earth coordinates
 *      - \f$ \vec{\xi}     \f$ = normalized direction
 *                                in spherical earth coordinates
 *                              = direction divided by speed of sound
 *      - \f$ \frac{d}{dt}_n\f$ = time derivative at step "n"
 *      - \f$ \Delta t      \f$ = marching algorithm time step
 *
 * The actual work of computing these derivatives and the effects of
 * the ocean environment is done by the wave_front class. The wave_front
 * entries are stored in a circular queue so that environmental
 * parameters computations and wavefront derivatives can be reused by
 * subsequent time steps.
 *
 * @xref S.M. Reilly, G. Potty, Sonar Propagation Modeling using Hybrid
 * Gaussian Beams in Spherical/Time Coordinates, January 2012.
 */
class USML_DECLSPEC wave_queue : public eigenray_notifier,
                                 public eigenverb_notifier,
                                 public reflection_notifier,
                                 public wave_thresholds {
    friend class reflection_model;
    friend class spreading_ray;
    friend class spreading_hybrid_gaussian;

   public:
    /**
     * Type of spreading model to be used.
     */
    typedef enum { CLASSIC_RAY, HYBRID_GAUSSIAN } spreading_type;

    //**************************************************
    // methods

    /**
     * Initialize a propagation scenario.
     *
     * @param  ocean        Reference to the environmental parameters.
     * @param  freq         Frequencies over which to compute propagation (Hz).
     * @param  pos          Location of the wavefront source in spherical
     *                      earth coordinates.
     * @param  de           Initial depression/elevation angles at the
     *                      source location (degrees, positive is up).
     *                      Requires a minimum of 3 angles if eigenrays
     *                      are being computed.
     * @param  az           Initial azimuthal angle at the source location
     *                      (degrees, clockwise from true north).
     *                      Requires a minimum of 3 angles if eigenrays
     *                      are being computed.
     *                      Ray fans that wrap around all azimuths should
     *                      include rays for both 0 and 360 degrees.
     * @param  time_step    Propagation step size (seconds).
     * @param  target_pos   List of acoustic target positions.
     * @param  type         Type of spreading model to use: CLASSIC_RAY
     *                      or HYBRID_GAUSSIAN.
     *   NOTE: The freq paramater above, is a seq_vector pointer and is owned
     *         by caller of the wave_queue constructor. It is the responsibilty
     *         of the caller to ensure the pointer is not freed from memory
     * while the wave_queue propagates and to delete the pointer when it is no
     *         longer required.
     */
    wave_queue(const ocean_model::csptr& ocean, const seq_vector::csptr& freq,
               const wposition1& pos, const seq_vector::csptr& de,
               const seq_vector::csptr& az, double time_step,
               const wposition* target_pos = nullptr,
               spreading_type type = HYBRID_GAUSSIAN);

    /** Destroy all temporary memory. */
    virtual ~wave_queue();

    /**
     * Location of the wavefront source in spherical earth coordinates.
     *
     * @return              Common origin of all points on the wavefront
     */
    inline const wposition1& source_pos() const { return _source_pos; }

    /**
     * List of frequencies for this wave queue.
     *
     * @return              Frequencies for this wavefront
     */
    inline seq_vector::csptr frequencies() const { return _frequencies; }

    /**
     * Initial depression/elevation angle at the source location.
     *
     * @param  de           Index of the element to access.
     * @return              Depression/elevation angle.
     *                      (degrees, positive is up)
     */
    inline double source_de(size_t de) const { return (*_source_de)(de); }

    /**
     * Initial azimuthal angle at the source location.
     *
     * @param  az           Index of the element to access.
     * @return              Depression/elevation angle.
     *                      (degrees, clockwise from true north)
     */
    inline double source_az(size_t az) const { return (*_source_az)(az); }

    /**
     * Elapsed time for the current element in the wavefront.
     */
    inline double time() const { return _time; }

    /**
     * Propagation step size (seconds).
     */
    inline double time_step() const { return _time_step; }

    /**
     * List of acoustic targets.
     */
    inline const wposition* targets() const { return _target_pos; }

    /**
     * Return next element in the wavefront.
     */
    inline const wave_front* next() { return _next; }

    /**
     * Return const next element in the wavefront.
     */
    inline const wave_front* next() const { return _next; }

    /**
     * Return current element in the wavefront.
     */
    inline const wave_front* curr() { return _curr; }

    /**
     * Return const current element in the wavefront.
     */
    inline const wave_front* curr() const { return _curr; }

    /**
     * Return previous element in the wavefront.
     */
    inline const wave_front* prev() { return _prev; }

    /**
     * Return const previous element in the wavefront.
     */
    inline const wave_front* prev() const { return _prev; }

    /**
     * Return past element in the wavefront.
     */
    inline const wave_front* past() { return _past; }

    /**
     * Return const past element in the wavefront.
     */
    inline const wave_front* past() const { return _past; }

    /**
     * Number of D/E angles in the ray fan.
     */
    inline size_t num_de() const { return _source_de->size(); }

    /**
     * Number of AZ angles in the ray fan.
     */
    inline size_t num_az() const { return _source_az->size(); }

    /**
     * Optional identification number for this wavefront calculation.
     * This can be used to correlate results to a specific wavefront when
     * concurrent wavefronts are used.  The scheme for defining this ID
     * can be different for each sonar training system.
     *
     * @param id    New identification number for this wavefront calculation.
     */
    inline void runID(size_t id) { _run_id = id; }

    /**
     * Optional identification number for this wavefront calculation.
     */
    inline const size_t runID() const { return _run_id; }

    /**
     * Marches to the next integration step in the acoustic propagation.
     * Uses the third order Adams-Bashforth algorithm to estimate the position
     * and direction of each point on the next wavefront from the previous
     * three iterations.  Automatically rotates the queue and updates the
     * environmental parameters on the new wavefront.
     *
     * Accumulated non-spreading losses are computed by combining the
     * individual values in the next wavefront with prior losses in
     * the current wavefront.
     *
     * If targets have been specified, this function calls detect_eigenrays()
     * at the end of each step to search for wavefront collisions with those
     * targets.
     *
     * At the end of each step, the next iteration may extend beyond one of
     * the boundaries.  This allows the eigenray calculation to accurately
     * portray targets near the interface.  Reflections are computed at the
     * beginning of the next iteration to ensure that the next wave elements
     * are alway inside of the water column.
     */
    void step();

   protected:
    /**
     * Reference to the environmental parameters.
     * Assumes that the storage for this data is managed by calling routine.
     */
    ocean_model::csptr _ocean;

    /**
     * Frequencies over which to compute propagation loss (Hz).
     * Defined as a pointer to support virtual methods in seq_vector class.
     */
    seq_vector::csptr _frequencies;

    /**
     * Location of the wavefront source in spherical earth coordinates.
     * Adjusted during construction of the wave_queue if it is within
     * 0.1 meters of being above the ocean surface or below the ocean bottom.
     * The boundary reflection logic does not perform correctly if the
     * wavefront starts on the wrong side of either boundary.
     */
    wposition1 _source_pos;

    /**
     * Initial depression/elevation angle (D/E) at the
     * source location (degrees, positive is up).
     * Defined as a pointer to support virtual methods in seq_vector class.
     */
    seq_vector::csptr _source_de;

    /**
     * Initial azimuthal angle (AZ) at the source location
     * (degrees, clockwise from true north).
     * Defined as a pointer to support virtual methods in seq_vector class.
     */
    seq_vector::csptr _source_az;

    /**
     * Maximum index for source_de
     */
    const size_t _max_de;

    /**
     * Maximum index for source_az
     */
    const size_t _max_az;

    /** Propagation step size (seconds). */
    double _time_step;

    /** Time for current entry in the wave_front circular queue (seconds). */
    double _time;

    /**
     * List of acoustic targets.
     */
    const wposition* _target_pos;

    /** Run Identification */
    size_t _run_id;

    /**
     * Intermediate term: sin of colatitude for targets.
     * By caching this value here, we avoid re-calculating it each time
     * the that wave_front::compute_target_distance() needs to
     * compute the distance squared from each target to each point
     * on the wavefront.
     */
    matrix<double> _targets_sin_theta;

    /** Reference to the reflection model component. */
    reflection_model* _reflection_model;

    /**
     * Reference to the spreading loss model component.
     * Supports either classic ray theory or Hybrid Gaussian Beams.
     */
    spreading_model* _spreading_model;

    /**
     * Circular queue of wavefront elements needed by the
     * third order Adams-Bashforth algorithm.
     *
     *    - _past is for iteration n-2
     *    - _prev is for iteration n-1
     *    - _curr is for iteration n (the current wavefront)
     *    - _next is for iteration n+1
     */
    wave_front *_past, *_prev, *_curr, *_next;

    /**
     * Create an Azimuthal boundary loop condition upon initialization.
     * This condition will prevent the production of multiple eigenrays
     * for instances where the first azimuthal angle is equivalent to
     * the last azimuthal angle in the AZ vector that is passed.
     */
    bool _az_boundary;

    /**
     * Treat all targets that are slightly away from directly above the
     * source as special cases.
     */
    bool _de_branch;

    /**
     * Initialize wavefronts at the start of propagation using a
     * 3rd order Runge-Kutta algorithm.  The Runge-Kutta algorithm is
     * much more computationally expensive than the Adams-Bashforth algorithm
     * used during propagation. But Runge-Kutta is self starting,
     * only happens at initialization, and it avoids introducing the start-up
     * errors that would be present with cheaper methods.
     *
     * Assumes that all of the elements of the _curr wavefront have been
     * initialized prior to this initialization. When this method is complete,
     * the wavefront elements for _past, _prev, _and _next will all have
     * valid position, direction, and closest point of approach data.
     * However, the _next element will not have been checked for interface
     * collisions or eigenray collisions with targets.
     */
    void init_wavefronts();

    //**************************************************
    // reflections and caustics

    /**
     * Detect and process boundary reflections and caustics.  Loops through all
     * of the "next" wavefront elements to see if any are on the wrong side of
     * a boundary.
     *
     * Relies on detect_reflections_surface() and detect_reflections_bottom()
     * to do the actual work of detecting and processing reflections.
     * These routines work recursively with their opposite so that multiple
     * reflections can take place in a single time step.  This is critical
     * in very shallow water where the reflected position may already be
     * beyond the opposing boundary.
     *
     * At the end of this process, the wave_front::find_edges()
     * routine is used to break the wavefront down into ray families.
     * A ray family is defined by a set of rays that have the same
     * surface, bottom, or caustic count.
     */
    void detect_reflections();

    /**
     * Detect and process surface reflection for a single (DE,AZ) combination.
     * The attenuation and phase of reflection loss are added to the
     * values currently being stored in the next wave element.
     * Works recursively with detect_reflections_bottom() so that multiple
     * reflections can take place in a single time step.
     *
     * @param   de      D/E angle index number.
     * @param   az      AZ angle index number.
     * @return        True if first recursion reflects from surface.
     */
    bool detect_reflections_surface(size_t de, size_t az);

    /**
     * Detect and process reflection for a single (DE,AZ) combination.
     * The attenuation and phase of reflection loss are added to the
     * values currently being stored in the next wave element.
     * Works recursively with detect_reflections_surface() so that multiple
     * reflections can take place in a single time step.
     *
     * @param   de      D/E angle index number.
     * @param   az      AZ angle index number.
     * @return        True if first recursion reflects from bottom.
     */
    bool detect_reflections_bottom(size_t de, size_t az);

    /**
     * Upper and lower vertices are present when the wavefront undergoes a
     * change in direction in the water column but does not interact with
     * the surface or bottom. A lower vertex is present if this point on the
     * wavefront is a local minimum in time. Conversely, an upper vertex
     * is present if it is a local maximum in time.
     */
    void detect_vertices(size_t de, size_t az);

    /**
     * Detects and processes all of the logic necessary to determine
     * points along the wavefronts that have folded over and mark them
     * as caustics. This logic determines if any two points have crossed
     * over each other when going from current wavefront to the next.
     */

    void detect_caustics(size_t de, size_t az);

    /**
     * Searches the volume layers collisions and sends data to the
     * reverberation model. Compares the rho coordinate of the curr
     * and next wavefronts to the height of each layer at the curr and next
     * locations.  Calls collide_from_above() if the curr point is above
     * but the next point is below the layer.  Calls collide_from_below()
     * if the curr point is below but the next point is above.
     */
    void detect_volume_scattering(size_t de, size_t az);

    //**************************************************
    // eigenray estimation routines

    /**
     * Detect and process wavefront closest point of approach (CPA) with target.
     * Requires a minimum of three rays in the D/E and AZ directions. Targets
     * beyond the edge of the wavefront are matched to the next ray inside
     * the fan.
     */
    void detect_eigenrays();

    /**
     * Used by detect_eigenrays() to discover if the current ray is the
     * closest point of approach (CPA) to the current target. Computes the
     * distance to each of the 27 neighboring wavefront points for later use
     * in the calculation of eigenray interpolation products.
     *
     * Exits early if the central ray is on the edge of a ray family.
     * Exits early if the distance to any of the surrounding points is
     * smaller than the distance to the central point, unless that point is on
     * the edge of the ray family, to which the search continues.
     * Ties are awarded to the higher time, higher D/E, and higher AZ.
     * Extrapolates outside of ray families by not testing to see if
     * points on the edge of the family are closer to the target than the
     * central ray.
     *
     * Assumes that the wavefront has three rays in the D/E and AZ directions.
     * Also assumes that the calling routine does not search the rays on the
     * edges of the wavefront.  This ensures that each of the tested rays has
     * valid rays to either side of it.
     *
     * @param   t1          Row number of the current target.
     * @param   t2          Column number of the current target.
     * @param   de          D/E angle index number.
     * @param   az          AZ angle index number.
     * @param   center      Reference to the center of the distance2 cube.
     * @param   distance2   Distance squared to each of the 27 neighboring
     *                      points. The first index is time, the second is D/E
     *                      and the third is AZ (output).
     * @return  True if central point is closest point of approach.
     */
    bool is_closest_ray(size_t t1, size_t t2, size_t de, size_t az,
                        const double& center, double distance2[3][3][3]);

    /**
     * Used by detect_eigenrays() to compute eigneray parameters and
     * add a new eigenray entry to the current target. Uses compute_offset()
     * to find offsets in time, source D/E, and source AZ that minimize
     * the distance to the target. This routine uses these offsets and
     * a Taylor series for direction to compute the target D/E and AZ.
     *
     * Intensity is computed using either a classic ray theory or a hybrid
     * Gaussian beam summation across the wavefront. Attenuation is
     * incorporated into the ray intensity using an interpolation in time
     * along the CPA ray. Phase is copied from the CPA as are the counts
     * for surface, bottom, and caustics.
     *
     * @param   t1          Row number of the current target.
     * @param   t2          Column number of the current target.
     * @param   de          D/E angle index number.  Can not equal a value
     *                      at the edge of the ray fan.
     * @param   az          AZ angle index number.  Can not equal a value
     *                      at the edge of the ray fan.
     * @param   distance2   Distance squared to each of the 27 neighboring
     *                      points. The first index is time, the second is D/E
     *                      and the third is AZ. Warning: this array
     *                      is modified during the computation.
     */
    void build_eigenray(size_t t1, size_t t2, size_t de, size_t az,
                        double distance2[3][3][3]);

    /**
     * Find relative offsets and true distances in time, D/E, and AZ.
     * Uses the analytic solution for the inverse of a symmetric 3x3 matrix
     * to solve
     * <pre>
     *              H x = g
     *              x = inv(H) g
     * </pre>
     * where
     *  - x is the relative offset,
     *  - g is the gradient (1st derviative) of the distances around CPA, and
     *  - H is the Hessian (2nd derviative) of the distances around CPA.
     *
     * If the determinant of the 3x3 matrix is very small, the inverse is
     * unstable, and this implementation falls back to a 2x2 calculation
     * in the time and AZ directions.  If the determinant of the 2x2 matrix
     * is also very small,  This implementation falls back to a solution that
     * just uses the diagonal of the Hessian.
     * <pre>
     *              x(n) = - g(n) / H(n,n)
     * </pre>
     * This fallback solution is limited to 1/2 of the beamwidth.  If this
     * clipping is not performed, the eigenray_extra_test/eigenray_lloyds
     * will generate significant errors in D/E.  But including this clipping
     * leads to large D/E errors when the target is outside of the ray fan,
     * like it is in the Bottom Bounce path for eigenray_test/eigenray_basic.
     *
     * Computes distances from offsets, for each coordinate, by assuming that
     * the other two offsets are zero.
     * <pre>
     *              d(n)^2 = - g(n) x(n) - 0.5*H(n,n) x(n)^2
     * </pre>
     * If this Taylor series around the CPA is unstable, this implementation
     * reverts to a simpler calculation that limits the inverse to the
     * time and AZ directions.  The neighborhood is treated as unstable if:
     *
     *  - The number of surface, bottom, and caustics is not the same for
     *    wavefront points around the CPA.
     *  - One of the wavefront points around the CPA is on the edge
     *    of the wavefront.
     *  - The determinant of the full Hesssian is very small, which indicates
     *    that the inverse unstable.
     *  - The DE offset computed by the full inverse is more than 50% of the
     *    beam width.
     *
     * If the neighborhood is unstable, the distance in the DE direction
     * is computed by subtracting the time and AZ distance from the total
     * distances at CPA.
     * <pre>
     *              d(DE)^2 = d(total)^2 - d(time)^2 - d(AZ)^2
     * </pre>
     *
     * @param   t1          Row number of the current target.
     * @param   t2          Column number of the current target.
     * @param   de          D/E angle index number.  Can not equal a value
     *                      at the edge of the ray fan.
     * @param   az          AZ angle index number.  Can not equal a value
     *                      at the edge of the ray fan.
     * @param   distance2   Distance squared to each of the 27 neighboring
     *                      points. The first index is time, the second is D/E
     *                      and the third is AZ.
     * @param   delta       Axis step size in each dimension. The first index
     *                      is time, the second is D/E and the third is AZ.
     * @param   offset      Distance to the current target in ray coordinate
     *                      units.  The first index is time offset, the second
     *                      is launch angle D/E offset, and the third is launch
     *                      angle AZ offset. Maximum limited to +/- 1 beam.
     *                      (output)
     * @param   distance    Distance to the current target in world coordinate
     *                      units (meters). Give the distance the same sign
     *                      as the relative offset. (output)
     */
    void compute_offsets(size_t t1, size_t t2, size_t de, size_t az,
                         const double distance2[3][3][3],
                         const c_vector<double, 3>& delta,
                         c_vector<double, 3>& offset,
                         c_vector<double, 3>& distance);

    /**
     * Computes the Taylor series coefficients used to compute eigenrays.
     * The vector Taylor series uses the first derivative (gradient)
     * and second derivative (Hessian) to estimate eigenray products
     * as a function of time, D/E, and AZ from the closest point
     * of approach (CPA).
     *
     * @param   value       Value to interpolate around the CPA.
     * @param   delta       Axis step size in each dimension. The first index
     *                      is time, the second is D/E and the third is AZ.
     * @param   center      Value at the center of the grid (output).
     * @param   gradient    First derivative in 3 dimensions (output).
     * @param   hessian     Second derivative in 3 dimensions (output).
     *
     * @xref Weisstein, Eric W. "Hessian." From MathWorld--A Wolfram Web
     *       Resource. http://mathworld.wolfram.com/Hessian.html
     */
    static void make_taylor_coeff(const double value[3][3][3],
                                  const c_vector<double, 3>& delta,
                                  double& center, c_vector<double, 3>& gradient,
                                  c_matrix<double, 3, 3>& hessian);

    /**
     * Computes a refined location and direction at the point of collision.
     * Uses a second order Taylor series around the current location to
     * estimate these values.
     *
     * @param de            D/E angle index number.
     * @param az            AZ angle index number.
     * @param time_water    The distance (in time) from the "current"
     *                      wavefront to the collision.
     * @param position      Refined position of the collision (output).
     * @param ndirection    Normalized direction at the point
     *                      of collision (output).
     * @param speed         Speed of sound at the point of collision (output).
     */
    void collision_location(size_t de, size_t az, double time_water,
                            wposition1* position, wvector1* ndirection,
                            double* speed) const;

    /**
     * Constructs an eigenverb from a collision with a boundary_model
     * or volume_model.  Uses the height and width of the beam at the
     * point of collision to estimate the length and width of the
     * eigerverb projection onto the interface.  Computes the initial
     * area, height, and width for the part of the wavefront centered
     * around each ray, using the increment halfway to next and
     * previous ray. These definitions support arbitrary ray spacing.
     * \f[
     *         A_{n,m} = \int_{\varphi_{m-1/2}}^{\varphi_{m+1/2}}
     *                   \int_{\mu_{n-1/2}}^{\mu_{n+1/2}}
     *                   \cos{\mu} \; d\mu \; d\varphi
     *             = \left[ \sin{ \mu_{n+1/2} } - \sin{ \mu_{n-1/2} } \right]
     *             \left[ \varphi_{m+1/2} - \varphi_{m-1/2} \right]
     * \f]\f[
     *         \Delta\mu_{n,m} = \mu_{n+1/2} - \mu_{n-1/2}
     * \f]\f[
     *         \Delta\varphi_{n,m} = \frac{ A_{n,m} }{ \Delta\mu_{n,m} }
     *            = \frac{ \sin{ \mu_{n+1/2} } - \sin{ \mu_{n-1/2} } }
     *              {  \mu_{n+1/2} - \mu_{n-1/2} }
     *               \left[ \varphi_{m+1/2} - \varphi_{m-1/2} \right]
     * \f]\f[
     *         \mu_{n+1/2} = \mu_{n,m} + 1/2 \; \delta\mu_{n}
     * \f]\f[
     *         \mu_{n-1/2} = \mu_{n,m} - 1/2 \; \delta\mu_{n-1}
     * \f]\f[
     *         \varphi_{n+1/2} = \varphi_{n,m} + 1/2 \; \delta\varphi_{m}
     * \f]\f[
     *         \varphi_{n-1/2} = \varphi_{n,m} - 1/2 \; \delta\varphi_{m-1}
     * \f]
     * where:
     *         - \f$ \mu_{n,m} \f$ = D/E launch angle for this ray;
     *         - \f$ \varphi_{n,m} \f$ = AZ launch angle for this ray;
     *         - \f$ \delta\mu_{n} \f$ = ray spacing in D/E direction;
     *         - \f$ \delta\varphi_{m} \f$ = ray spacing in AZ direction.
     *         - \f$ A_{n,m} \f$ = surface area for this ray at 1 meter from
     * source;
     *         - \f$ \Delta\mu_{n,m} \f$ = average height of this surface area;
     *         - \f$ \Delta\varphi_{n,m} \f$ = average width of this surface
     * area;
     *
     * The eigenverb is the projection of a Gaussian intensity profile onto
     * the interface. This implementation computes the length and width of the
     * eigenverb, assuming that the change is proportional to path length.
     * \f[
     *         L_{n,m,k} = \ell_k \; \Delta\mu_{n,m} / \sin{ \gamma_k }
     * \f]\f[
     *         W_{n,m,k} = \ell_k \; \Delta\varphi_{n,m}
     * \f]\f[
     *         P_{n,m,k} = \alpha_{n,m} \; A_{n,m} / \sin{ \gamma_k }
     * \f]
     * where:
     *        - k = collision number;
     *        - \f$ L_{n,m,k} \f$ = Length of eigenverb along the interface;
     *        - \f$ W_{n,m,k} \f$ = Width of eigenverb along the interface;
     *        - \f$ P_{n,m,k} \f$ = Total power in eigenverb integrated over
     * interface.
     *
     * If the eigenverb meets the intensity threshold, the eigenverb is passed
     * to the collision listener who then calls its collector to save the
     * eigenverb.
     *
     * Because this can be an expensive calculation, it includes logic to
     * exits immediately if any of the following conditions are met.
     *
     * - An eigenverb listener has not yet been defined.
     * - The number of surface, bottom, caustic, upper, lower bounces
     *   exceeds the current threshold.
     * - The wave has not started to propagate.
     * - The grazing angle is less than 1e-6 radians.
     * - The last azimuth of the fan overlaps the first azimuth.
     * - The launch D/E angle is greater +/- 89.9 degrees.  This excludes
     *   the region where initial area covered by ray is close to zero.
     *
     * It will also exit prematurely if the power is below
     * the eigenverb threshold.
     *
     * @param de            D/E angle index number.
     * @param az            AZ angle index number.
     * @param dt            Offset in time to collision with the boundary
     * @param grazing       The grazing angle at point of impact (rads)
     * @param speed         Speed of sound at the point of collision.
     * @param position      Location at which the collision occurs
     * @param ndirection    Normalized direction at the point of collision.
     * @param type          Interface number for the interface that generated
     *                         for this eigenverb.  See the eigenverb_collection
     *                         class header for documentation on interpreting
     *                         this number. For some layers, you can also use
     * the eigenverb::interface_type.
     */
    void build_eigenverb(size_t de, size_t az, double dt, double grazing,
                         double speed, const wposition1& position,
                         const wvector1& ndirection, size_t type);

    //**************************************************
    // wavefront_netcdf routines

   private:
    /**
     * The netCDF file used to record the wavefront log.
     * Reset to nullptr when not initialized.
     */
    NcFile* _nc_file;

    /** The netCDF variables used to record the wavefront log. */
    NcVar *_nc_time, *_nc_latitude, *_nc_longitude, *_nc_altitude, *_nc_surface,
        *_nc_bottom, *_nc_caustic, *_nc_upper, *_nc_lower, *_nc_on_edge;

    /** Current record number in netDCF file. */
    long _nc_rec;

   public:
    /**
     * Initialize recording to netCDF wavefront log.
     * Opens the file and records initial conditions.
     * The file structure is illustrated by the netCDF sample below:
     * <pre>
     *   netcdf sample_test {
     *   dimensions:
     *          frequency = 1 ;
     *          source_de = 25 ;
     *          source_az = 9 ;
     *          travel_time = UNLIMITED ; // (### currently)
     *   variables:
     *          double frequency(frequency) ;
     *                  frequency:units = "hertz" ;
     *          double source_de(source_de) ;
     *                  source_de:units = "degrees" ;
     *                  source_de:positive = "up" ;
     *          double source_az(source_az) ;
     *                  source_az:units = "degrees_true" ;
     *                  source_az:positive = "clockwise" ;
     *          double travel_time(travel_time) ;
     *                  travel_time:units = "seconds" ;
     *          double latitude(travel_time, source_de, source_az) ;
     *                  latitude:units = "degrees_north" ;
     *          double longitude(travel_time, source_de, source_az) ;
     *                  longitude:units = "degrees_east" ;
     *          double altitude(travel_time, source_de, source_az) ;
     *                  altitude:units = "meters" ;
     *                  altitude:positive = "up" ;
     *          short surface(travel_time, source_de, source_az) ;
     *                  surface:units = "count" ;
     *          short bottom(travel_time, source_de, source_az) ;
     *                  bottom:units = "count" ;
     *          short caustic(travel_time, source_de, source_az) ;
     *                  caustic:units = "count" ;
     *          short upper(travel_time, source_de, source_az) ;
     *                  caustic:units = "count" ;
     *          short lower(travel_time, source_de, source_az) ;
     *                  caustic:units = "count" ;
     *          byte on_edge(travel_time, source_de, source_az) ;
     *                  caustic:units = "bool" ;
     *
     *   // global attributes:
     *                  :Conventions = "COARDS" ;
     *
     *   data:
     *      frequency = 2000 ;
     *      source_de = 0, 1, 2, ...
     *      source_az = -1, 0, 1 ;
     *      travel_time = 0, 0.1, 0.2, ...
     *      latitude = 45, 45, 45, ...
     *      longitude = -45, -45, -45, ...
     *      altitude = -75, -75, -75, ...
     *      etc...
     *   }
     * </pre>
     * @param   filename    Name of the file to write to disk.
     * @param   long_name   Optional global attribute for identifying data-set.
     */
    void init_netcdf(const char* filename, const char* long_name = nullptr);

    /**
     * Write current record to netCDF wavefront log.
     * Records travel time, latitude, longitude, altitude for
     * the current wavefront.
     */
    void save_netcdf();

    /**
     * Close netCDF wavefront log.
     */
    void close_netcdf();
};

/// @}
}  // end of namespace waveq3d
}  // end of namespace usml
