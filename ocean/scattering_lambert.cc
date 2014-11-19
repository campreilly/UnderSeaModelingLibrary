/**
 * @file scattering_lambert.cc
 */

#include <usml/ocean/scattering_lambert.h>

using namespace usml::ocean ;

/**
 * Computes the broadband scattering strength for a single location.
 */
void scattering_lambert::scattering_strength( const wposition1& location,
    const seq_vector& frequencies, double de_incident, double de_scattered,
    double az_incident, double az_scattered, vector<double>* amplitude )
{
    (*amplitude)(0) = abs( _bss * sin( de_incident ) * sin( de_scattered ) ) ;
    for(unsigned i=1; i<frequencies.size(); ++i) {
        (*amplitude)(i) = (*amplitude)(0) ;
    }
}
