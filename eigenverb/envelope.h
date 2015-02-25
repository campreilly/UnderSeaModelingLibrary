/**
 * @file envelope_collection.h
 */
#pragma once

#include <boost/numeric/ublas/vector_expression.hpp>

namespace usml {
namespace eigenverb {

using boost::numeric::ublas::vector ;

/**
 *
 */
struct envelope {

    /**
     * Radial index for this envelope
     */
    size_t  azimuth ;

    /**
     * Duration of the reverberation envelope
     */
    double time ;

    /**
     * Reverberation envelope
     */
    vector<double> intensity ;

};

/**
 * List of envelopes
 */
typedef vector<envelope>   envelope_collection ;

}   // end of namespace eigenverb
}   // end of namespace usml
