/** 
 * @file reflect_loss_constant.cc
 * Models reflection loss as a constant factor.
 */
#include <usml/ocean/reflect_loss_constant.h>

using namespace usml::ocean ;
          
/**
 * Computes the broadband reflection loss and phase change.
 */
void reflect_loss_constant::reflect_loss( 
    const wposition1& location, 
    const seq_vector& frequencies, double angle,
    vector<double>* amplitude, vector<double>* phase )
{
    for ( unsigned f=0 ; f < frequencies.size() ; ++f ) {
        (*amplitude)(f) = _amplitude ;
        if ( phase ) (*phase)(f) = _phase ;
    }
}
