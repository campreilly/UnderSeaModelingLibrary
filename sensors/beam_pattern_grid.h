/**
 * @file beam_pattern_grid.h
 */
#pragma once

#include <usml/sensors/beam_pattern_model.h>
#include <usml/types/data_grid.h>
#include <algorithm>

namespace usml {
namespace sensors {

using boost::numeric::ublas::vector ;
using namespace usml::types ;

/// @ingroup beams
/// @{

/**
 * A beam pattern function as a mesh or grid beam pattern.
 */
template<class T, std::size_t Dim>
class beam_pattern_grid: public beam_pattern_model, public data_grid<T,Dim> {

    public:

        typedef T               value_type ;
        typedef T*              value_ptr ;
        typedef value_ptr*      value_double_ptr ;
        typedef std::size_t     size_type ;
        typedef beam_pattern_grid<T,Dim>    self_type ;
        typedef enum { LINEAR_UNITS, LOG_UNITS }    data_units ;

        /**
         * Constructs a grid or mesh of beam levels for a beam
         * pattern. The axes order is dimension zero, frequencies
         * dimension one, DEs and dimension two, AZs. The data
         * is passed in as a pointer along with a data_unit.
         * The construct pattern call then determines how to
         * interpret the data passed in and adjusts it as needed.
         *
         * Once the data grid is constructed, the directivity index
         * is computed. This uses the analytic definition of
         * directivity index and then stores these values as a
         * function of frequency in a separate data_grid.
         *
         * @param axes          list of axes for the beam pattern
         * @param data          pointer to the beam levels
         * @param data_unit     units that the data are in upon
         *                      being passed in.
         */
        beam_pattern_grid( seq_vector* axes[], const value_ptr data,
                           const data_units& data_unit )
            : data_grid<T,Dim>( axes )
        {
            construct_pattern( data, data_unit ) ;
            construct_directivity_grid() ;
            _beamID = beam_pattern_model::GRID ;
        }

        /**
         * Destructor
         */
        virtual ~beam_pattern_grid() {
            if( _directivity_index ) {
                delete _directivity_index ;
                _directivity_index = NULL ;
            }
        }

        /**
         * Computes the beam level gain along a specific DE and AZ direction.
         *
         * @param de            Depression/Elevation angle (rad)
         * @param az            Azimuthal angle (rad)
         * @param orient        Orientation of the array
         * @param frequencies   List of frequencies to compute beam level for
         * @param level         Beam level for each frequency (linear units)
         */
        virtual void beam_level( double de, double az,
                                 orientation& orient,
                                 const vector<double>& frequencies,
                                 vector<double>* level)
        {
            size_type num_freq( frequencies.size() ) ;
            vector<double> tmp( num_freq, 0.0 ) ;
            switch( Dim ) {

                /**
                 * 1-D gridded beam levels
                 */
                case 1 :
                    for(size_type i=0; i<num_freq; ++i) {
                        value_type freq = frequencies[i] ;
                        tmp[i] = this->interpolate( &freq ) ;
                    }
                    noalias(*level) = tmp ;
                    break ;

                /**
                 * 2-D gridded beam levels
                 */
				case 2:
					{
						value_type location[2] ;
						double de_prime = 0 ;
						double dummy = 0 ;
						orient.apply_rotation( de, az, &de_prime, &dummy ) ;
						location[1] = de_prime ;
						for (size_type i = 0; i < num_freq; ++i) {
							location[0] = frequencies[i] ;
							tmp[i] = this->interpolate(location) ;
						}
						noalias(*level) = tmp ;
					}
                    break ;

                /**
                 * 3-D gridded beam levels
                 */
				case 3:
					{
						value_type location[3] ;
                        double de_prime = 0 ;
                        double az_prime = 0 ;
                        orient.apply_rotation( de, az, &de_prime, &az_prime ) ;
						location[1] = de_prime ;
						location[2] = az_prime ;
						for (size_type i = 0; i < num_freq; ++i) {
							location[0] = frequencies[i] ;
							tmp[i] = this->interpolate(location) ;
						}
						noalias(*level) = tmp ;
					}
                    break ;

                /**
                 * Invalid dimension value
                 */
                default :
                    std::invalid_argument(
                            "beam_pattern_grid must be 1-D, 2-D, or 3-D") ;
                    break ;
            }
        }

        /**
         * Computes the directivity index for each frequency.
         */
        virtual void directivity_index( const vector<double>& frequencies,
                                        vector<double>* level )
        {
            size_type num_freq( frequencies.size() ) ;
            vector<double> tmp( num_freq, 0.0 ) ;
            switch( Dim ) {

                /**
                 * 1-D gridded beam levels
                 */
                case 1 :
                    for(size_type i=0; i<num_freq; ++i) {
                        value_type freq = frequencies[i] ;
                        value_type di = _directivity_index->interpolate( &freq ) ;
                        tmp[i] = 10.0 * log10( di ) ;
                    }
                    break ;

                /**
                 * 2-D gridded beam levels
                 */
                case 2 :
                    for(size_type i=0; i<num_freq; ++i) {
                        value_type freq = frequencies[i] ;
                        value_type di = _directivity_index->interpolate( &freq ) ;
                        tmp[i] = 10.0 * log10( 2.0 / di ) ;
                    }
                    break ;

                /**
                 * 3-D gridded beam levels
                 */
                case 3 :
                    for(size_type i=0; i<num_freq; ++i) {
                        value_type freq = frequencies[i] ;
                        value_type di = _directivity_index->interpolate( &freq ) ;
                        tmp[i] = 10.0 * log10( (4.0*M_PI) / di ) ;
                    }
                    break ;

                /**
                 * Invalid dimension value
                 */
                default :
                    std::invalid_argument(
                            "beam_pattern_grid must be 1-D, 2-D, or 3-D") ;
                    break ;
            }
            noalias(*level) = tmp ;
        }


    private:

        /**
         * Data grid that stores the directivity index.
         */
        data_grid<double,1>* _directivity_index ;

        /**
         * Constructs the data grid using the passed in data.
         */
        void construct_pattern( const value_ptr data,
                                const data_units& data_unit )
        {
            size_type N = 1 ;
            for(size_type i=0; i<Dim; ++i) {
                N *= this->_axis[i]->size() ;
                this->interp_type( i, GRID_INTERP_PCHIP ) ;
            }
            switch( data_unit ) {

                /**
                 * Data that has been passed in is in log units
                 * and needs to be converted to linear units
                 */
                case LOG_UNITS :
                {
                    value_ptr data_copy = new value_type[N] ;
                    memcpy( data_copy, data, N*sizeof(value_type) ) ;
                    for(size_type i=0; i<N; ++i) {
                        *data_copy = std::pow( 10.0, (-(*data_copy)/10.0) ) ;
                        ++data_copy ;
                    }
                    memcpy( this->_data, data_copy, N*sizeof(value_type) ) ;
                    delete data_copy ;
                    break ;
                }

                /**
                 * The data was already passed to this constructor in
                 * linear units.
                 */
                default:
                    memcpy( this->_data, data, N*sizeof(value_type) ) ;
                    break ;
            }
            memset( this->_edge_limit, true, Dim*sizeof(bool) ) ;
        }

       /**
        * Computes the directivity index by summing all beam
        * level contributions for each frequency along every
        * DE and AZ. Stores the gird locally to be used
        * on call.
        */
       void construct_directivity_grid() {
           size_type num_freq( this->_axis[0]->size() ) ;
           seq_vector* axes[] = { this->axis(0) } ;
           _directivity_index = new data_grid<double,1>( axes ) ;
           value_ptr p_data = _directivity_index->data() ;
           for(size_type i=0; i<num_freq; ++i) {
               sum_data( i, *(p_data++) ) ;
           }
       }

       /**
        * Adds all intensities for a specific frequency.
        *
        * @param index      Index for the frequency of interest
        * @param total      Reference to return the total values
        * @return           The summation of all intensities for
        *                   this frequency
        */
       void sum_data( const size_type index, value_type& total )
       {
           total = 0.0 ;
           switch( Dim ) {

               /**
                * 1-D grid of data points
                */
               case 1 :
                   total = this->_data[index] ;
                   break ;

               /**
                * 2-D grid of data points
                */
               case 2 :
               {
                   size_type num_de( this->_axis[1]->size() ) ;
                   seq_vector* theta = this->_axis[1] ;
                   for(size_type i=0; i<num_de; ++i) {
                       total += this->_data[index*num_de+i] * cos( (*theta)[i] )
                                * theta->increment(i) ;
                   }
                   break ;
               }

               /**
                * 3-D grid of data points
                */
               case 3 :
               {
                   size_type num_de( this->_axis[1]->size() ) ;
                   seq_vector* theta = this->_axis[1] ;
                   size_type num_az( this->_axis[2]->size() ) ;
                   seq_vector* phi = this->_axis[2] ;
                   for(size_type i=0; i<num_de; ++i) {
                       for(size_type j=0; j<num_az; ++j) {
                           size_type tmp = j + num_az * (i + num_de*index) ;
                           total += this->_data[tmp] * cos( (*theta)[i] )
                                   * theta->increment(i) * phi->increment(j) ;
                       }
                   }
                   break ;
               }

               /**
                * Invalid dimension value
                */
               default:
                   std::invalid_argument(
                           "beam_pattern_grid must be 1-D, 2-D, or 3-D") ;
                   break ;
           }
       }

};

/// @}
}   // end of namespace sensors
}   // end of namespace usml
