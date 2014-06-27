/**
 * @file eigenverb_bistatic.cc
 */

#include <usml/waveq3d/eigenverb_bistatic.h>
#include <usml/waveq3d/spreading_hybrid_gaussian.h>

using namespace usml::waveq3d ;

/**  Constructor  **/
eigenverb_bistatic::eigenverb_bistatic( wave_queue& wave ) {
    _spreading_model = wave.getSpreading_Model() ;
}

/**
 * Places an eigenverb into the class of "upper" bins to be used for the overall
 * reverberation calculation.
 */
void eigenverb_bistatic::notifyUpperCollision( unsigned de, unsigned az, double time,
               double dt, double grazing, double speed, const seq_vector& frequencies,
               const wposition1& position, const wvector1& ndirection, int ID ) {

    eigenverb verb ;
    verb.de = de ;
    verb.az = az ;
    verb.time = time + dt ;
    verb.grazing = grazing ;
    verb.c = speed ;
    verb.pos = position ;
    verb.ndir = ndirection ;
    verb.frequencies = &frequencies ;

        // Calculate the one way TL and the width of the gaussian
        // at the time of impact with the boundary.
    vector<double> offset(3) ;
    vector<double> distance(3) ;
    offset(0) = dt ;                                      // Temporal offset still exists
    offset(1) = offset(2) = 0.0 ;                           // No offset in DE and AZ
    distance(0) = distance(1) = distance(2) = 0.0 ;         // Zero distance
    const vector<double> amp = _spreading_model->getIntensity( position, de, az, offset, distance ) ;
    verb.intensity = - 10.0 * log10( amp ) ;
    verb.sigma_de = _spreading_model->getWidth_DE( de, az, offset ) ;
    verb.sigma_az = _spreading_model->getWidth_AZ( de, az, offset ) ;

    switch (ID) {
        // Interactions from the volume reverberation will use
        // IDs to distinguish where they will be cataloged to.
        case 11:
            _source_surface.push_back( verb ) ;
            break ;
        case 21:
            _receiver_surface.push_back( verb ) ;
            break ;
        default:
            break ;
    }
}

/**
 * Places an eigenverb into the class of "lower" bins to be used for the overall
 * reverberation calculation.
 */
void eigenverb_bistatic::notifyLowerCollision( unsigned de, unsigned az, double time,
               double dt, double grazing, double speed, const seq_vector& frequencies,
               const wposition1& position, const wvector1& ndirection, int ID ) {

    eigenverb verb ;
    verb.de = de ;
    verb.az = az ;
    verb.time = time + dt ;
    verb.grazing = grazing ;
    verb.c = speed ;
    verb.pos = position ;
    verb.ndir = ndirection ;
    verb.frequencies = &frequencies ;

        // Calculate the one way TL and the width of the gaussian
        // at the time of impact with the boundary.
    c_vector<double,3> offset, distance ;
    offset(0) = dt ;                                      // Temporal offset still exists
    offset(1) = offset(2) = 0.0 ;                           // No offset in DE and AZ
    distance(0) = distance(1) = distance(2) = 0.0 ;         // Zero distance
    const vector<double> amp = _spreading_model->getIntensity( position, de, az, offset, distance ) ;
    verb.intensity = - 10.0 * log10( amp ) ;
    verb.sigma_de = _spreading_model->getWidth_DE( de, az, offset ) ;
    verb.sigma_az = _spreading_model->getWidth_AZ( de, az, offset ) ;


    switch (ID) {
        // Interactions from the volume reverberation will use
        // IDs to distinguish where they will be cataloged to.
        case 10:
            _source_bottom.push_back( verb ) ;
            break ;
        case 20:
            _receiver_bottom.push_back( verb ) ;
            break ;
        default:
            break ;
    }
}

void eigenverb_bistatic::compute_reverberation() {}
