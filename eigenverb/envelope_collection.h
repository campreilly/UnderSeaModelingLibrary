/**
 * @file envelope_collection.h
 */
#pragma once

#include <cstddef>
#include <usml/ublas/vector_math.h>
#include <boost/foreach.hpp>

using boost::numeric::ublas::vector ;
using namespace usml::ublas ;

namespace usml {
namespace eigenverb {

class USML_DECLSPEC envelope_collection {

    public:

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

}   // end of namespace eigenverb
}   // end of namespace usml
