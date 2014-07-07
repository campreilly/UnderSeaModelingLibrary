/**
 * @file scattering_lambert.cc
 */

#include <usml/ocean/scattering_lambert.h>

using namespace usml::ocean ;

void scattering_lambert::scattering_strength( const wposition1& location,
    const seq_vector& frequencies, double angleI, double angleS,
    double azI, double azS, vector<double>* amplitude,
    vector<double>* phase )
{
    double coeff = 1.0 ;
    for(unsigned i=0; i<frequencies.size(); ++i) {
        (*amplitude)(i) = coeff * sin( angleI ) * sin( angleS ) ;       /// @todo define coeff I_0*dA
    }
}
