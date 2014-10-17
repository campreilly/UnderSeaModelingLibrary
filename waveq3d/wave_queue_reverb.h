/**
 * @file wave_queue_reverb.h
 */

#ifndef USML_WAVEQ3D_WAVE_QUEUE_REVERB_H
#define USML_WAVEQ3D_WAVE_QUEUE_REVERB_H

#include <usml/waveq3d/wave_queue.h>
#include <usml/waveq3d/reverberation_model.h>
#include <usml/utilities/SharedPointerManager.h>

namespace usml {
namespace waveq3d {

class reflection_model ;
class spreading_model ;

/**
 * ID value used for source and receivers.
 */
enum ORIGIN { SOURCE_ID = 10, RECEIVER_ID = 20 } ;

class USML_DECLSPEC wave_queue_reverb : public wave_queue {

    public:

        typedef usml::utilities::SharedPointerManager<reverberation_model>      Pointer_Manager ;

        /**
         * Initialize a propagation scenario.
         *
         * @param  ocean        Reference to the environmental parameters.
         * @param  freq         Frequencies over which to compute propagation (Hz).
         * @param  pos          Location of the wavefront source in spherical
         *                      earth coordinates.
         * @param  de           Initial depression/elevation angles at the
         *                      source location (degrees, positive is up).
         * @param  az           Initial azimuthal angle at the source location
         *                      (degrees, clockwise from true north).
         *                      Ray fans that wrap around all azimuths should
         *                      include rays for both 0 and 360 degrees.
         * @param  time_step    Propagation step size (seconds).
         * @param  reverb       Pointer to reverberation model used for this wave_queue
         * @param  targets      List of acoustic targets.
         * @param  type         Type of spreading model to use: CLASSIC_RAY
         *                      or HYBRID_GAUSSIAN.
         */
        wave_queue_reverb(
            ocean_model& ocean,
            const seq_vector& freq,
            const wposition1& pos,
            const seq_vector& de,
            const seq_vector& az,
            double time_step,
            const wposition* targets=NULL,
            spreading_type type=HYBRID_GAUSSIAN ) ;

        /** Destroy all temporary memory. */
        virtual ~wave_queue_reverb() {}

        /**
         * Sets the reverberation model for the wave_queue.
         */
        void setReverberation_Model( Pointer_Manager m ) ;

        /**
         * Returns the number of frequencies in _frequencies.
         */
        const unsigned getFreqSize() ;

        /**
         * Accessor to validate a ray for reverberation contributions
         */
        virtual bool is_ray_valid( unsigned de, unsigned az ) {
            return !_invalid_ray(de,az) ;
        }

    protected:

        /**
         * Marks rays within the ray fan that are not to be valid rays that can contribute
         * to the overall contributions of the reverberation level. These rays are not
         * valid because of the limitations of the model producing a valid spreading
         * loss for these rays using the, at this time, available spreading models.
         */
        matrix<bool> _invalid_ray ;

//        /**
//         * Marks all rays to either be valid rays to contribute to reverberation levels
//         * or not. All rays at time zero are invalid because of the zero distance between
//         * them initially, and after time zero, only rays that are _source_de(0), last
//         * two _source_de's and last two _source_az's.
//         */
//        void mark_invalid_rays() ;

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
         * Specialized call within wave_queue reverberation calculations. This call
         * searches the volume layers of the ocean for layer collisions and sends the
         * appropriate data to the reverberation model to be used in volume reverberation
         * contributions.
         *
         * The function checks every point along the wavefront if the current altitude
         * and the next time step altitude of the point cross a layer. If a point crosses
         * a boundary in a time step, collide_from_above or collide_from_below are called.
         */
        void detect_volume_reflections() ;

        /**
         * A modified version of the function reflection_model::bottom_reflection used
         * to determine the infromation needed to produce a volume reverberation calculation
         * from this layer when colliding from above the layer.
         */
        void collide_from_above( unsigned de, unsigned az, double depth, unsigned layer ) ;

        /**
         * A modified version of the function reflection_model::surface_reflection used
         * to determine the infromation needed to produce a volume reverberation calculation
         * from this layer when colliding from below the layer.
         *      @todo need to rectify this code, it mimics the bottom_reflection but signs
         *            should be changed to adjust for approaching from below.
         */
        void collide_from_below( unsigned de, unsigned az, double depth, unsigned layer ) ;

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

};

}   // end of namespace waveq3d
}   // end of namespace usml

#endif
