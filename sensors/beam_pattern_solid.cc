/**
 * @file beam_pattern_solid.cc
 */

#include <usml/sensors/beam_pattern_solid.h>

using namespace usml::sensors ;

/** Calculates the beam level in de, az, and frequency **/
void beam_pattern_solid::beam_level(
        double de, double az,
        double theta, double phi,
        const vector<double>& frequencies,
        vector<double>* level )
{
	write_lock_guard(_mutex);
//	double theta_prime = M_PI_2 + de ;
    double up = _max_de - theta ;
    double down = _min_de - theta ;
    double left = _min_az + phi ;
    double right = _max_az + phi ;
    vector<double> result = scalar_vector<double>( frequencies.size(), 0.0 ) ;
    if( (de <= up) && (de >= down) ) {
        if( (az <= right) && (az >= left) ) {
            result = scalar_vector<double>( frequencies.size(), 1.0 ) ;
        }
    }
    noalias(*level) = result ;
}

/**
 * Computes the directivity index
 */
void beam_pattern_solid::directivity_index(
        const vector<double>& frequencies,
        vector<double>* level )
{
	write_lock_guard(_mutex);
    noalias(*level) =
            scalar_vector<double>( frequencies.size(), _directivity_index ) ;
}
/**
 * Initializes the beam pattern
 */
void beam_pattern_solid::initialize_beam()
{
    _max_de *= M_PI / 180.0 ;
//    _max_de += M_PI / 2 ;
    _min_de *= M_PI / 180.0 ;
//    _min_de += M_PI / 2 ;
    _max_az *= M_PI / 180.0 ;
    _min_az *= M_PI / 180.0 ;
    double solid = (_max_az - _min_az)*(sin(_max_de) - sin(_min_de)) ;
    _directivity_index = 10.0*log10( (4.0*M_PI) / solid ) ;
}
