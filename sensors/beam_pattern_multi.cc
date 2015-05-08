/**
 * @file beam_pattern_multi.cc
 */

#include <usml/sensors/beam_pattern_multi.h>
#include <boost/foreach.hpp>

using namespace usml::sensors ;
using namespace boost ;

/**
 * Constructor
 */
beam_pattern_multi::beam_pattern_multi(
    std::list<beam_pattern_model*> beam_list )
    : _beam_list(beam_list)
{

}

/**
 * Destructor
 */
beam_pattern_multi::~beam_pattern_multi()
{
    BOOST_FOREACH( beam_pattern_model* b, _beam_list )
        delete b ;
}

/**
 * Multiplies all beam levels from each beam pattern
 */
void beam_pattern_multi::beam_level(
    double de, double az,
    orientation& orient,
    const vector<double>& frequencies,
    vector<double>* level )
{
    write_lock_guard(_mutex) ;
    vector<double> tmp( frequencies.size(), 1.0 ) ;
    noalias(*level) = vector<double>( frequencies.size(), 1.0 ) ;
    BOOST_FOREACH( beam_pattern_model* b, _beam_list )
    {
        b->beam_level( de, az, orient, frequencies, &tmp ) ;
        *level = element_prod( *level, tmp ) ;
    }
}

/**
 * Multiplies the directivity indices from each beam pattern
 */
void beam_pattern_multi::directivity_index(
    const vector<double>& frequencies,
    vector<double>* level )
{
    write_lock_guard(_mutex) ;
    vector<double> tmp( frequencies.size(), 1.0 ) ;
    noalias(*level) = vector<double>( frequencies.size(), 1.0 ) ;
    BOOST_FOREACH( beam_pattern_model* b, _beam_list )
    {
        b->directivity_index( frequencies, &tmp ) ;
        tmp = pow( 10.0, tmp ) ;
        *level = element_prod( *level, tmp ) ;
    }
    *level = 10.0 * log10( *level ) ;
}
