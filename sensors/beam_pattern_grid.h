/**
 * @file beam_pattern_grid.h
 */
#pragma once

#include <usml/sensors/beam_pattern_model.h>
#include <algorithm>

namespace usml {
namespace sensors {

using boost::numeric::ublas::vector ;
using namespace usml::types ;

/// @ingroup beams
/// @{

/**
 * A beam pattern function as a gridded solution.
 */
template<class T, std::size_t Dim>
class beam_pattern_grid: public beam_pattern_model, public data_grid<T,Dim> {

    public:

        typedef T               value_type ;
        typedef T*              value_ptr ;
        typedef pointer*        value_double_ptr ;
        typedef std::size_t     size_type ;
        typedef enum { LINEAR, LOG, DATA }          axis_type ;
        typedef enum { LINEAR_UNITS, LOG_UNITS }    data_units ;
        typedef enum { DEGREES, RADIANS }           axis_units ;

        /**
         * Constructor
         *
         * @param axis
         * @param data
         * @param types
         * @param axes_units
         * @param data_unit
         */
        beam_pattern_grid( const value_double_ptr axis, const value_ptr data,
                           const axis_type* types, const axis_units* axes_units,
                           const data_units& data_unit )
        {
            construct_pattern( axis, data, types, axes_units, data_unit ) ;
            compute_directivity_index() ;
        }

        /**
         * Computes the beam level gain along a specific DE and AZ direction.
         *
         * @param  de            Depression/Elevation angle (rad)
         * @param  az            Azimuthal angle (rad)
         * @param  beam          beam steering to find response level (size_t)
         * @param  level         beam level for each frequency
         */
        virtual void beam_level( double de, double az, size_t beam,
                                 vector<double>* level )
        {
            size_type num_freq( _axis[0]->size() ) ;
            value_type location[Dim] ;
            location[1] = de ;
            if( Dim > 2 ) {
                location[2] = az ;
            }
            scalar_vector<double> tmp( num_freq, 0.0 ) ;
            for(size_type i=0; i<size; ++i) {
                location[0] = (_axis[0])[i] ;
                tmp[i] = interpolate( location ) ;
            }
            noalias(*level) = tmp ;
        }

        /**
         * Rotates the array by a given roll, pitch, and yaw
         *
         * @param roll      rotation of the beam around the North/South axis (up positive)
         * @param pitch     rotation of the beam around the East/West axis (up positive)
         * @param yaw       rotation of the beam around the Up/Down axis (clockwise positive)
         */
        virtual void orient_beam( double roll, double pitch, double yaw ) ;

    private:

        /**
         *
         */
        void construct_grid( const value_double_ptr axis, const value_ptr data,
                             const axis_type* type, const axis_units* axes_units,
                             const data_units& data_unit )
        {
            size_type N = 1 ;
            for(size_type i=0; i<Dim; ++i) {
                size_type size = sizeof(axis[i])/sizeof(value_type) ;
                N *= size ;
                this->_axis[i] = create_axis( axis[i], size, type[i], axes_units[i] ) ;
                interp_type( i, GRID_INTERP_LINEAR ) ;
            }
            this->_data = new value_type[N] ;
            switch( data_unit ) {
                case LOG_UNITS:
                    value_ptr data_copy ;
                    memcpy( data_copy, data, N*sizeof(value_type) ) ;
                    for(size_type i=0; i<N; ++i) {
                        (*data_copy++) = std::pow( 10.0, (-data_copy/10.0) ) ;
                    }
                    memcpy( this->_data, data_copy, N*sizeof(value_type) ) ;
                    break ;
                default:
                    memcpy( this->_data, data, N*sizeof(value_type) ) ;
                    break ;
            }
            memset( this->_edge_limit, true, Dim*sizeof(bool) ) ;
        }

        /**
         * Create a seq_vector from axis data.
         *
         * @param axis_data     Pointer to the data in the axis
         * @param size          Size of the axis
         * @param type          Type of seq_vector to be constructed
         * @param units         Type of units the axis is in
         * @return              a constructed seq_vector
         */
        seq_vector* create_axis( const value_ptr axis_data,
                                 const size_type& size,
                                 const axis_type& type,
                                 const axis_units& units )
        {
            value_type inc = axis_data[1] - axis_data[0] ;
            value_type factor = 1.0 ;
            if( units == DEGREES ) {
                factor = M_PI / 180.0 ;
            }
            switch( type ) {
                case LINEAR:
                    return new seq_linear( axis_data[0]*factor, inc*factor, size ) ;
                    break ;
                case LOG:
                    return new seq_log( axis_data[0], inc, size ) ;
                    break ;
                default:
                    value_ptr data_copy ;
                    std::memcpy( data_copy, axis_data, size*sizeof(value_type) ) ;
                    for(size_type i=0; i<size; ++i) {
                        (*data_copy++) *= factor ;
                    }
                    return new seq_data( data_copy, size ) ;
                    break ;
            }
        }

       /**
        * Computes the directivity index for each frequency.
        */
       void compute_directivity_index()
       {
           size_type num_freq( _axis[0].size() ) ;
           _directivity_index =
               scalar_matrix<double>( num_freq, 0.0 ) ;
           size_type num( sizeof(data)/sizeof(value_type) ) ;
           value_type total = 2.0 ;
           if( Dim > 2 ) {
               total = 4.0*M_PI ;
           }
           for(size_type i=0; i<num_freq; ++i) {
               value_type sum = sum_data( this->_data, i ) ;
               _directivity_index[i] = 10.0 * log10( total / sum ) ;
           }
       }

       /**
        * Adds all intensities for a specific frequency.
        *
        * @param index      Index for the frequency of interest
        * @return           The summation of all intensities for
        *                   this frequency
        */
       void sum_data( const size_type index, value_type& total )
       {
           switch( Dim ) {

                   /**
                    * 1-D grid of data points
                    */
               case 1 :
                   total = _data[index] ;
                   break ;

                   /**
                    * 2-D grid of data points
                    */
               case 2 :
                   total = 0.0 ;
                   size_type size( _axis[1]->size() ) ;
                   value_ptr dtheta = &(_axis[1]->increment(0)) ;
                   value_ptr theta = _axis[1]->begin() ;
                   for(size_type i=0; i<size; ++i) {
                       total += _data[index+i] * cos( (*theta+i) ) * (*dtheta+i) ;
                   }
                   break ;

                   /**
                    * 3-D grid of data points
                    */
               case 3 :
                   total = 0.0 ;
                   size_type size1( _axis[1]->size() ) ;
                   size_type size2( _axis[2]->size() ) ;
                   value_ptr theta = _axis[1]->begin() ;
                   value_ptr dtheta = &(_axis[1]->increment(0)) ;
                   value_ptr dphi = &(_axis[2]->increment(0)) ;
                   for(size_type i=0; i<size1; ++i) {
                       for(size_type j=0; j<size2; ++j) {
                           total += _data[index+i*size1+j*size2] * cos( (*theta+i) )
                                   * (*dtheta+i) * (*dphi+i) ;
                       }
                   }
                   break ;

                   /**
                    * Invalid dimension value
                    */
               default:
                   std::invalid_argument(
                           "beam_pattern_grid must be 1-D, 2-D, or 3-D") ;
                   break ;
           }
           size_type N = 1 ;
           value_type result = 0.0 ;
           for(size_type i=1; i<Dim; ++i) {
               N *= _axis[i]->size() ;
           }
       }

};

/// @}
}   // end of namespace sensors
}   // end of namespace usml
