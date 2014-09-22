/**
 * @file eigenverb_model.h
 */

#ifndef USML_WAVEQ3D_EIGENVERB_MODEL_H
#define USML_WAVEQ3D_EIGENVERB_MODEL_H

//#define EIGENVERB_COLLISION_DEBUG
//#define EIGENVERB_MODEL_DEBUG

#include <usml/ocean/ocean.h>
#include <usml/ublas/math_traits.h>
#include <usml/waveq3d/eigenverb.h>
#include <usml/waveq3d/reverberation_model.h>
#include <usml/waveq3d/wave_queue_reverb.h>
#include <usml/waveq3d/spreading_model.h>
#include <usml/ublas/ublas.h>
#include <boost/numeric/ublas/lu.hpp>
#include <usml/types/types.h>
#include <vector>
#include <iostream>
#include <fstream>

namespace usml {
namespace waveq3d {

using namespace usml::ocean ;

using namespace boost::numeric::ublas ;

class USML_DECLSPEC eigenverb_model : public reverberation_model {

    public:

        /**
         * Virtual destructor
         */
        virtual ~eigenverb_model() {}

        /**
         * React to the collision of a single ray with a reverberation
         * when colliding from below the boundary.
         *
         * @param de            D/E angle index number.
         * @param az            AZ angle index number.
         * @param dt            Offset in time to collision with the boundary
         * @param grazing       The grazing angle at point of impact (rads)
         * @param speed         Speed of sound at the point of collision.
         * @param position      Location at which the collision occurs
         * @param ndirection    Normalized direction at the point of collision.
         * @param wave          Wave queue, used to extract various data
         * @param ID            (Used to identify source/receiver/volume layer)
         */
        virtual void notifyUpperCollision( unsigned de, unsigned az,
               double dt, double grazing, double speed,
               const wposition1& position, const wvector1& ndirection,
               const wave_queue& wave, unsigned ID ) {}

        /**
         * React to the collision of a single ray with a reverberation
         * when colliding from above the boundary.
         *
         * @param de            D/E angle index number.
         * @param az            AZ angle index number.
         * @param dt            Offset in time to collision with the boundary
         * @param grazing       The grazing angle at point of impact (rads)
         * @param speed         Speed of sound at the point of collision.
         * @param position      Location at which the collision occurs
         * @param ndirection    Normalized direction at the point of collision.
         * @param wave          Wave queue, used to extract various data
         * @param ID            (Used to identify source/receiver/volume layer)
         */
        virtual void notifyLowerCollision( unsigned de, unsigned az,
               double dt, double grazing, double speed,
               const wposition1& position, const wvector1& ndirection,
               const wave_queue& wave, unsigned ID ) {}

        /**
         * Computes the reverberation curve from the data cataloged from the
         * wavefront(s).
         */
        virtual void compute_reverberation() {
            compute_bottom_energy() ;
            compute_surface_energy() ;
            compute_upper_volume_energy() ;
            compute_lower_volume_energy() ;
        }

        /**
         * Gains access to the reverberation data. The user should first execute
         * compute_reverberation() prior to requesting access to the entire
         * reverberation curve.
         * @return      pointer to _reverberation_curve
         */
         virtual const vector<double> getReverberation_curve() {
            return _reverberation_curve ;
         }

         /**
          * Saves the eigenverb data to a text file.
          */
         virtual void save_eigenverbs(const char* filename) {}

    protected:

        /**
         * Constructs and eigenverb from the data provided in a notify collision.
         *
         * @param de            D/E angle index number.
         * @param az            AZ angle index number.
         * @param dt            Offset in time to collision with the boundary
         * @param grazing       The grazing angle at point of impact (rads)
         * @param speed         Speed of sound at the point of collision.
         * @param position      Location at which the collision occurs
         * @param ndirection    Normalized direction at the point of collision.
         * @param wave          Wave queue, used to extract various data
         * @return verb         newly constructed eigenverb
         */
        void create_eigenverb( unsigned de, unsigned az,
               double dt, double grazing, double speed,
               const wposition1& position, const wvector1& ndirection,
               const wave_queue& wave, eigenverb& verb ) const ;

        /**
         * Computes the contribution value of two eigenverbs to the total
         * reverberation level.
         */
        void compute_contribution( const eigenverb* u, const eigenverb* v ) ;

        /**
         * Computes the energy contributions to the reverberation
         * energy curve from the bottom interactions.
         */
        virtual void compute_bottom_energy() = 0 ;

        /**
         * Computes the energy contributions to the reverberation
         * energy curve from the surface interactions.
         */
        virtual void compute_surface_energy() = 0 ;

        /**
         * Calculate the contributions due to collisions from below
         * a volume layer.
         */
        virtual void compute_upper_volume_energy() = 0 ;

        /**
         * Calculate the contributions due to collisions from above
         * a volume layer.
         */
        virtual void compute_lower_volume_energy() = 0 ;

        /**
         * Pulse length of the signal (sec)
         */
        double _pulse ;

        /**
         * Number of time bins to store the energy distribution
         */
        unsigned _max_index ;

        /**
         * Max time for the reverberation curve
         */
        double _max_time ;

        /**
         * Number of layers within the volume
         */
        unsigned _n ;

        /**
         * Origin ID of the source wavefront
         */
        unsigned _source_origin ;

        /**
         * Origin ID of the reciever wavefront
         */
        unsigned _receiver_origin ;

        /**
         * Defines the type of spreading model that is used to compute
         * one-way TLs and sigma of each dimension.
         */
        spreading_model* _spreading_model ;

        /**
         * Defines the type of boundary model for the bottom.
         */
        boundary_model* _bottom_boundary ;

        /**
         * Defines the type of boundary model for the surface.
         */
        boundary_model* _surface_boundary ;

        /**
         * Defines the type(s) of boundary model(s) for the volume.
         */
        volume_layer* _volume_boundary ;

        /**
         * The current boundary that computations need
         */
        boundary_model* _current_boundary ;

        /**
         * The reverberation energy distribution curve. The values in this
         * array are in linear units.
         */
        vector<double> _reverberation_curve ;

        /**
         * Time resolution of the reverberation curve. This is used to spread
         * the energy from a contribution out in time along the curve.
         */
        vector<double> _two_way_time ;

        /****/
        vector<double> __receiver_latitude ;

        /****/
        vector<double> __receiver_longitude ;

        /****/
        vector<double> __receiver_az ;

        /****/
        vector<double> __receiver_grazing ;

        /****/
        vector<double> __Wr ;

        /****/
        vector<double> __Lr ;

        /****/
        vector<double> __receiver_loss ;

        /****/
        vector<double> __receiver_launch_az ;

        /****/
        vector<double> __receiver_time ;

};

}   // end of namespace waveq3d
}   // end of namespace usml

#endif
