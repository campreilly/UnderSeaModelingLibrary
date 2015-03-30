/**
 * @file envelope_collection.h
 */
#pragma once

#include <cstddef>
#include <usml/eigenverb/eigenverb.h>
#include <usml/types/quad.h>
#include <usml/ublas/vector_math.h>
#include <boost/foreach.hpp>
#include <netcdfcpp.h>

using boost::numeric::ublas::vector ;
using namespace usml::ublas ;

namespace usml {
namespace eigenverb {

class USML_DECLSPEC envelope_collection {

    public:

    // shared pointer
    typedef boost::shared_ptr<envelope_collection> reference;

        /**
         * Constructor
         *
         * @param resolution
         * @param num_bins
         * @param num_az
         */
        envelope_collection(
                double resolution,
                size_t num_bins,
                size_t num_az ) ;

        /**
         * Destructor
         */
        virtual ~envelope_collection() ;

        /**
         * Initializes the envelope and two way travel time
         * containers that are used to compute and add
         * Guassian projections.
         *
         * @param size
         * @param resolution
         */
        void initialize( size_t size, double resolution ) ;

        /**
         * Adds a gaussian contribution to the time series,
         * reverberation envelope, along the azimuthal direction.
         *
         * @param peak
         * @param travel_time
         * @param duration_time
         * @param az
         */
        void add_gaussian( double peak, double travel_time,
                           double duration_time, size_t az ) ;

        /**
         * Returns the reverberation envelope along a specific
         * azimuthal direction.
         *
         * @param az
         */
         vector<double> envelopes( size_t az ) const ;

         /**
          * Returns the entire reveberation envelope collection
          */
         vector<vector<double>*> envelopes() const ;

         /**
          * Writes the envelope data to disk
          */
         void write_netcdf( const char* filename ) ;

    private:

         /**
          * Reverberation envelope collection
          */
         vector<vector<double>*> _envelopes ;

         /**
          * Reveberation time duration
          */
         vector<double> _two_way_time ;

};

/**
 * Eigenverb box used for querying a quadtree
 * with eigenverbs as the data.
 */
struct USML_DECLSPEC eigenverb_box {
    typedef double      coord_type ;

    /**
     * Construct a box from an eigenverb
     */
    eigenverb_box( const eigenverb& e )
    : x( e.position.longitude() ),
      y( e.position.latitude() )
    {
        width = std::abs(e.sigma_az * cos( e.launch_az )
                        - e.sigma_de * sin( e.launch_az ) ) ;
        height = std::abs(e.sigma_de * cos( e.launch_az )
                        + e.sigma_az * sin( e.launch_az ) ) ;
    }

    /**
     * Construct a box from a quadtree node
     */
    eigenverb_box( const usml::types::quad<eigenverb>& q )
    : x( q._x ),
      y( q._y ),
      width( q._w ),
      height( q._h )
    {}

    /**
     * Empty Constructor, canvases the entire Earth
     */
    eigenverb_box() : x( -M_PI_2 ), y( 0.0 ),
            width( 2.0*M_PI ), height( M_PI )
    {}

    /**
     * Public data members for the box
     */
    coord_type x ;
    coord_type y ;
    coord_type width ;
    coord_type height ;
};

}   // end of namespace eigenverb
}   // end of namespace usml
