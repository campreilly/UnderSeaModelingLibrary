/**
 * @file envelope_collection.h
 */
#pragma once

#include <cstddef>
#include <boost/numeric/ublas/vector_expression.hpp>

using boost::numeric::ublas::vector ;

namespace usml {
namespace eigenverb {

class USML_DECLSPEC envelope_collection {

    public:

        /**
         *
         */
        typedef vector<double>      envelope ;
        typedef vector<envelope>    collection ;

        /**
         * Constructor
         */
        envelope_collection() {}

        /**
         * Destructor
         */
        virtual ~envelope_collection() {}

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
         const envelope envelope( size_t az ) {
            return (*_envelopes)(az) ;
         }

         /**
          * Returns the entire reveberation envelope collection
          */
         const collection* envelope() {
             return _envelopes ;
         }

    private:

         /**
          * Reverberation envelope collection
          */
         collection* _envelopes ;

         /**
          * Reveberation time duration
          */
         vector<double> _two_way_time ;

};

}   // end of namespace eigenverb
}   // end of namespace usml
