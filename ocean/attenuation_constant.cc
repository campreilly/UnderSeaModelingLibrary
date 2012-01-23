/** 
 * @file attenuation_constant.cc
 * Models attenuation loss as a constant factor.
 */
 
#include <usml/ocean/attenuation_constant.h>

using namespace usml::ocean ;
          
/**
 * Computes the broadband absorption loss of sea water.
 */
void attenuation_constant::attenuation( 
    const wposition& location, 
    const seq_vector& frequencies,
    const matrix<double>& distance,
    matrix< vector<double> >* attenuation ) 
{
    for ( unsigned row=0 ; row < location.size1() ; ++row ) {
        for ( unsigned col=0 ; col < location.size2() ; ++col ) {
            for ( unsigned f=0 ; f < frequencies.size() ; ++f ) {
                (*attenuation)(row,col)(f) = 
                    _coefficient * distance(row,col) * frequencies(f) ;
            }
        }
    }
}
