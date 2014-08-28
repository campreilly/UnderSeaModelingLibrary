/**
 * @file scattering_lambert.cc
 */

#include <usml/ocean/scattering_lambert.h>

using namespace usml::ocean ;

/**
 * Computes a single scattering strength
 */
void scattering_lambert::scattering_strength( const wposition1& location,
    const seq_vector& frequencies, double angleI, double angleS,
    double azI, double azS, vector<double>* amplitude,
    vector<double>* phase )
{
    (*amplitude)(0) = _bss * sin( angleI ) * sin( angleS ) ;
    for(unsigned i=1; i<frequencies.size(); ++i) {
        (*amplitude)(i) = (*amplitude)(0) ;
    }
}

/**
 * Computes a vector of scattering strengths at multiple locations
 */
void scattering_lambert::scattering_strength( const wposition& location,
    const seq_vector& frequencies, const vector<double>& angleI,
    const vector<double>& angleS, const vector<double>& azI,
    const vector<double>& azS, vector<vector<double> >* amplitude,
    vector<vector<double> >* phase )
{
    (*amplitude)(0) = _bss * element_prod( sin( angleI ), sin( angleS ) ) ;
    for(unsigned i=1; i<frequencies.size(); ++i) {
        (*amplitude)(i) = (*amplitude)(0) ;
    }
}
