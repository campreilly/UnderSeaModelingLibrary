/**
 * @file wave_queue.h
 * Wavefront propagator for the WaveQ3D model.
 */
#pragma once

#include <usml/ocean/ocean.h>
#include <usml/waveq3d/wave_front.h>
#include <usml/waveq3d/eigenrayListener.h>
#include <netcdfcpp.h>

namespace usml {
namespace waveq3d {

using namespace usml::ocean ;
class reflection_model ;
class spreading_model ;
class spreading_ray ;
class spreading_hybrid_gaussian ;
class eigenrayListener ;

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
class USML_DECLSPEC wave_queue {

    friend class reflection_model ;
    friend class spreading_ray ;
    friend class spreading_hybrid_gaussian ;

  public:

    /**
     * Type of spreading model to be used.
     */
    typedef enum { CLASSIC_RAY, HYBRID_GAUSSIAN } spreading_type ;

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
     * @param  targets      List of acoustic targets.
     * @param  run_id		Run Identification number.
     * @param  type         Type of spreading model to use: CLASSIC_RAY
     *                      or HYBRID_GAUSSIAN.
     */
    wave_queue(
        ocean_model& ocean,
        const seq_vector& freq,
        const wposition1& pos,
        const seq_vector& de, const seq_vector& az,
        double time_step,
        const wposition* targets=NULL,
        const size_t run_id=1,
        spreading_type type=HYBRID_GAUSSIAN
        ) ;

    /** Destroy all temporary memory. */
    virtual ~wave_queue() ;

    /**
     * Location of the wavefront source in spherical earth coordinates.
     *
     * @return              Common origin of all points on the wavefront
     */
    inline const wposition1& source_pos() const {
        return _source_pos ;
    }

    /**
     * List of frequencies for this wave queue.
     *
     * @return              Frequencies for this wavefront
     */
    inline const seq_vector* frequencies() const {
        return _frequencies ;
    }

    /**
     * Initial depression/elevation angle at the source location.
     *
     * @param  de           Index of the element to access.
     * @return              Depression/elevation angle.
     *                      (degrees, positive is up)
     */
    inline double source_de( size_t de ) const {
        return (*_source_de)(de) ;
    }

    /**
     * Initial azimuthal angle at the source location.
     *
     * @param  az       	Index of the element to access.
     * @return              Depression/elevation angle.
     *                      (degrees, clockwise from true north)
     */
    inline double source_az( size_t az ) const {
        return (*_source_az)(az) ;
    }

    /**
     * Elapsed time for the current element in the wavefront.
     */
    inline double time() const {
        return _time ;
    }

    /**
     * Return next element in the wavefront.
     */
    inline const wave_front* next() {
        return _next ;
    }

    /**
     * Return const next element in the wavefront.
     */
    inline const wave_front* next() const {
        return _next ;
    }

    /**
     * Return current element in the wavefront.
     */
    inline const wave_front* curr() {
        return _curr ;
    }

    /**
     * Return const current element in the wavefront.
     */
    inline const wave_front* curr() const {
        return _curr ;
    }

    /**
     * Return previous element in the wavefront.
     */
    inline const wave_front* prev() {
        return _prev ;
    }

    /**
     * Return const previous element in the wavefront.
     */
    inline const wave_front* prev() const {
        return _prev ;
    }

    /**
     * Return past element in the wavefront.
     */
    inline const wave_front* past() {
        return _past ;
    }

    /**
     * Return const past element in the wavefront.
     */
    inline const wave_front* past() const {
        return _past ;
    }

    /**
     * Number of D/E angles in the ray fan.
     */
    inline size_t num_de() const {
        return _source_de->size() ;
    }

    /**
     * Number of AZ angles in the ray fan.
     */
    inline size_t num_az() const {
        return _source_az->size() ;
    }

    /**
	 * setIntensityThreshold
	 * @param  dThreshold The new value of the intensity threshold in dB.
	 *
	 */
	inline void setIntensityThreshold(double dThreshold) {

	    // Convert to absolute value for later comparison
	    // with the positive ray.intensity value.
		_intensity_threshold = abs(dThreshold);
	}
	/**
	 * getIntensityThreshold
	 * @return  Returns current value of the intensity threshold in dB
	 */
	inline double getIntensityThreshold() {
		return _intensity_threshold;
	}

    /**
     * Add a eigenrayListener to the _eigenrayListenerVec vector
     */
    bool addEigenrayListener(eigenrayListener* pListener);

    /**
	 * Remove a eigenrayListener from the _eigenrayListenerVec vector
	 */
    bool removeEigenrayListener(eigenrayListener* pListener);

    /**
	 * For each eigenrayListener in the _eigenrayListenerVec vector
	 * call the checkEigenrays method to deliver all eigenrays after
	 * a certain amount of time has passed.
	 *  @param	waveTime Current Time of the WaveFront in msec
	 *  @return True on success, false on failure.
	 */
	bool checkEigenrayListeners(long waveTime);

    /**
     * Set the type of wavefront that this is, i.e. a wavefront
     * originating from a source or receiver. This is exclusively
     * used within the reverbation models.
     */
    inline void setID( size_t id ) {
        _run_id = id ;
    }

    /**
     * Get the type of wavefront that this is, i.e. a wavefront
     * originating from a source or receiver. This is exclusively
     * used within the reverbation models.
     * @return      Type of wavefront (receiver/source)
     */
    inline const size_t getID() {
        return _run_id ;
    }

    /**
     * Protoype of the function that is needed during reflections
     * and only implemented in reverberation wave_queues.
     */
    virtual bool is_ray_valid( size_t de, size_t az ) {
        return false ;
    }

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
    void step() ;

  protected:

    /**
     * Reference to the environmental parameters.
     * Assumes that the storage for this data is managed by calling routine.
     */
    ocean_model& _ocean ;

    /**
     * Frequencies over which to compute propagation loss (Hz).
     * Defined as a pointer to support virtual methods in seq_vector class.
     */
    const seq_vector* _frequencies ;

    /**
     * Location of the wavefront source in spherical earth coordinates.
     */
    const wposition1 _source_pos ;

    /**
     * Initial depression/elevation angle (D/E) at the
     * source location (degrees, positive is up).
     * Defined as a pointer to support virtual methods in seq_vector class.
     */
    const seq_vector *_source_de ;

    /**
     * Initial azimuthal angle (AZ) at the source location
     * (degrees, clockwise from true north).
     * Defined as a pointer to support virtual methods in seq_vector class.
     */
    const seq_vector *_source_az ;

    /** Propagation step size (seconds). */
    double _time_step ;

    /** Time for current entry in the wave_front circular queue (seconds). */
    double _time ;

    /**
     * List of acoustic targets.
     */
    const wposition* _targets;

     /** Run Identification */
    size_t _run_id ;

	/**
	 * Intermediate term: sin of colatitude for targets.
	 * By caching this value here, we avoid re-calculating it each time
	 * the that wave_front::compute_target_distance() needs to
	 * compute the distance squared from each target to each point
	 * on the wavefront.
	 */
	matrix<double> _targets_sin_theta ;

    /** Reference to the reflection model component. */
    reflection_model* _reflection_model ;

    /**
     * Reference to the spreading loss model component.
     * Supports either classic ray theory or Hybrid Gaussian Beams.
     */
    spreading_model* _spreading_model ;

    /**
     * The value of the intensity threshold in dB
     * Any eigenray intensity values that are weaker than this
     * threshold are not sent the proplossListner(s);
     * Defaults to -300 dB
     */
    double _intensity_threshold; //In dB

    /**
     * Circular queue of wavefront elements needed by the
     * third order Adams-Bashforth algorithm.
     *
     *    - _past is for iteration n-2
     *    - _prev is for iteration n-1
     *    - _curr is for iteration n (the current wavefront)
     *    - _next is for iteration n+1
     */
    wave_front *_past, *_prev, *_curr, *_next ;


    /**
	* Vector containing the references of objects that will be used to
	* update classes that require eigenrays as they are built.
	* These classes must implement addEigenray method.
	*/
    std::vector<eigenrayListener *> _eigenrayListenerVec;

    /**
     * Create an Azimuthal boundary loop condition upon initialization.
     * This condition will prevent the production of multiple eigenrays
     * for instances where the first azimuthal angle is equivalent to
     * the last azimuthal angle in the AZ vector that is passed.
     */
    bool _az_boundary ;

    /**
     * Treat all targets that are slightly away from directly above the
     * source as special cases.
     */
    bool _de_branch ;

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
    void init_wavefronts() ;

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
    virtual void detect_reflections() ;

    /**
     * Detect and process surface reflection for a single (DE,AZ) combination.
     * The attenuation and phase of reflection loss are added to the
     * values currently being stored in the next wave element.
     * Works recursively with detect_reflections_bottom() so that multiple
     * reflections can take place in a single time step.
     *
     * @param   de      D/E angle index number.
     * @param   az      AZ angle index number.
     * @return		True if first recursion reflects from surface.
     */
    bool detect_reflections_surface( size_t de, size_t az ) ;

    /**
     * Detect and process reflection for a single (DE,AZ) combination.
     * The attenuation and phase of reflection loss are added to the
     * values currently being stored in the next wave element.
     * Works recursively with detect_reflections_surface() so that multiple
     * reflections can take place in a single time step.
     *
     * @param   de      D/E angle index number.
     * @param   az      AZ angle index number.
     * @return		True if first recursion reflects from bottom.
     */
    bool detect_reflections_bottom( size_t de, size_t az ) ;

    /**
     * Upper and lower vertices are present when the wavefront undergoes a
     * change in direction in the water column but does not interact with
     * the surface or bottom. A lower vertex is present if this point on the
     * wavefront is a local minimum in time. Conversely, an upper vertex
     * is present if it is a local maximum in time.
     */
    void detect_vertices( size_t de, size_t az ) ;

    /**
     * Detects and processes all of the logic necessary to determine
     * points along the wavefronts that have folded over and mark them
     * as caustics. This logic determines if any two points have crossed
     * over each other when going from current wavefront to the next.
     */

    void detect_caustics( size_t de, size_t az ) ;

    //**************************************************
    // eigenray estimation routines

    /**
     * Detect and process wavefront closest point of approach (CPA) with target.
     * Requires a minimum of three rays in the D/E and AZ directions. Targets
     * beyond the edge of the wavefront are matched to the next ray inside
     * the fan.
     */
    void detect_eigenrays() ;

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
    bool is_closest_ray(
        size_t t1, size_t t2,
        size_t de, size_t az,
        const double &center,
        double distance2[3][3][3] ) ;

    /**
     * Used by detect_eigenrays() to compute eigneray parameters and
     * add a new eigenray entry to the current target. Inverts
     * the 3-D Taylor series to find offsets in time, source D/E,
     * and source AZ that minimize the distance to the target.
     * \f[
     *      d^2( \vec{\rho} ) = c + \vec{b} \cdot \vec{\rho}
     *          + \frac{1}{2} \vec{\rho} \cdot A \cdot \vec{\rho}
     * \f]\f[
     *      \frac{ \delta d^2 }{ \delta \vec{\rho} } =
     *          \vec{b} + A \cdot \vec{\rho} = 0
     * \f]\f[
     *      A \cdot \vec{\rho} = - \vec{b}
     * \f]\f[
     *      \vec{\rho} = - A^{-1} \vec{b}
     * \f]
     * where:
     *      - \f$ \vec{ \rho } \f$ = offsets in time, D/E, AZ
     *      - \f$ A          \f$ = Hessian matrix (2nd derivative)
     *      - \f$ \vec{b}    \f$ = gradient vector (1st derivative)
     *      - \f$ c          \f$ = constant
     *
     * This routine uses these offsets and a Taylor series for direction to
     * compute the target D/E and AZ. It avoids extrapolating source and
     * target angles too far into shadow zone by limiting the D/E and AZ offsets
     * to the width of one beam.
     *
     * Intensity is computed using either a classic ray theory or a hybrid
     * Gaussian beam summation across the wavefront. Attenuation is
     * incorporated into the ray intensity using an interpolation in time
     * along the CPA ray. Phase is copied from the CPA as are the counts
     * for surface, bottom, and caustics.
     *
     * There is an interesting degenerate case that happens for targets
     * near the source location.  The CPA calculation from detect_eigenrays()
     * creates non-physical eigenrays around the point at which the 2nd
     * wavefront transitions from occurring before surface reflection to
     * including just after it. This routines examines the offsets in time and
     * ensonified area to eliminate these non-physical eigenrays.
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
    void build_eigenray(
        size_t t1, size_t t2,
        size_t de, size_t az,
        double distance2[3][3][3] ) ;

    /**
	 * For each eigenrayListener in the _eigenrayListenerVec vector
	 * call the addEigenray method to provide eigenrays to object that requested them.
	 */
	bool notifyEigenrayListeners(size_t targetRow, size_t targetCol, eigenray pEigenray);

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
     * This implementation reverts to a simplier calculation using just
     * the diagonals of the Hessian if the inverse can not be computed.
     *
     * Compute distances from offsets, for each coordinate, by assuming that
     * the other two offsets are zero.
     * <pre>
     *              d(n)^2 = - g(n) x(n) - 0.5*H(n,n) x(n)^2
     * </pre>
     * The DE distance calculations can be unstable if the target is far
     * from CPA.  This routine deals with this by computing the DE distance
     * from the time and AZ distances.
     * <pre>
     *              d(DE)^2 = d(total)^2 - d(time)^2 - d(AZ)^2
     * </pre>
     *
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
     * @param   unstable    True if full inverse is expected to be unstable.
     *                      May be updated by this routine if target is far
     *                      outside of ray fan. (input/output)
     */
    static void compute_offsets(
        const double distance2[3][3][3], const c_vector<double,3>& delta,
        c_vector<double,3>& offset, c_vector<double,3>& distance,
        bool& unstable ) ;

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
     * @param   diagonal_only   Zeros out Hessian cross terms when true.
     *
     * @xref Weisstein, Eric W. "Hessian." From MathWorld--A Wolfram Web
     *       Resource. http://mathworld.wolfram.com/Hessian.html
     */
    static void make_taylor_coeff(
        const double value[3][3][3], const c_vector<double,3>& delta,
        double& center, c_vector<double,3>& gradient, c_matrix<double,3,3>& hessian,
        bool diagonal_only = false ) ;

    //**************************************************
    // wavefront_netcdf routines

  private:

    /**
     * The netCDF file used to record the wavefront log.
     * Reset to NULL when not initialized.
     */
    NcFile* _nc_file ;

    /** The netCDF variables used to record the wavefront log. */
    NcVar *_nc_time, *_nc_latitude, *_nc_longitude, *_nc_altitude,
          *_nc_surface, *_nc_bottom, *_nc_caustic, *_nc_upper,
          *_nc_lower, *_nc_on_edge ;

    /** Current record number in netDCF file. */
    long _nc_rec ;

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
     *          short upper_vertex(travel_time, source_de, source_az) ;
     *                  caustic:units = "count" ;
     *          short lower_vertex(travel_time, source_de, source_az) ;
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
    void init_netcdf( const char* filename, const char* long_name=NULL ) ;

    /**
     * Write current record to netCDF wavefront log.
     * Records travel time, latitude, longtiude, altitude for
     * the current wavefront.
     */
    void save_netcdf() ;

    /**
     * Close netCDF wavefront log.
     */
    void close_netcdf() ;

};

/// @}
}  // end of namespace waveq3d
}  // end of namespace usml
