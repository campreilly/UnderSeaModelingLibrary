/**
 * @file eigenverb_monostatic.cc
 */

#include <usml/waveq3d/eigenverb_monostatic.h>
#include <usml/waveq3d/spreading_hybrid_gaussian.h>

using namespace usml::waveq3d ;

/**  Constructor  **/
eigenverb_monostatic::eigenverb_monostatic(
    wave_queue& wave, double pulse, unsigned num_bins, double max_time ) :
    _pulse(pulse),
    _num_bins(num_bins),
    _max_time(max_time)
{
    _spreading_model = wave.getSpreading_Model() ;
    _energy = new double[_num_bins] ;
    memset(_energy,0.0,_num_bins) ;
}

/** Destructor **/
eigenverb_monostatic::~eigenverb_monostatic()
{
    delete _energy ;
}
/**
 * Places an eigenverb into the class of "upper" bins to be used for the overall
 * reverberation calculation.
 */
void eigenverb_monostatic::notifyUpperCollision( unsigned de, unsigned az, double time,
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
        // No ID present, means this is a generic surface interaction.
        case 0:
            _surface.push_back( verb ) ;
            break ;
        // Interactions from the volume reverberation will use
        // IDs to distinguish where they will be cataloged to.
        default:
//            _upper(ID).push_back( verb ) ;
            break ;
    }
}

/**
 * Places an eigenverb into the class of "lower" bins to be used for the overall
 * reverberation calculation.
 */
void eigenverb_monostatic::notifyLowerCollision( unsigned de, unsigned az, double time,
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
    verb.intensity = amp ;
    verb.sigma_de = _spreading_model->getWidth_DE( de, az, offset ) ;
    verb.sigma_az = _spreading_model->getWidth_AZ( de, az, offset ) ;

    switch (ID) {
        // No ID present, means this is a generic bottom interaction.
        case 0:
            _bottom.push_back( verb ) ;
            break ;
        // Interactions from the volume reverberation will use
        // IDs to distinguish where they will be cataloged to.
        default:
//            _lower(ID).push_back( verb ) ;
            break ;
    }
}

void eigenverb_monostatic::compute_reverberation() {

    // define a 2pi squared for use multiple times
    double TWO_PI_2 = TWO_PI * TWO_PI ;

    // Convolution of all bottom paths
    for(std::vector<eigenverb>::iterator i=_bottom.begin();
            i!=_bottom.end(); ++i)
    {
            // Gather data for the out path to the bottom
        eigenverb u = (*i) ;
        matrix<double> mu1(2,1) ;
        matrix<double> sigma1(2,2) ;
        mu1(0,0) = u.pos.latitude() ;
        mu1(1,0) = u.pos.longitude() ;
        sigma1(0,1) = sigma1(1,0) = 0.0 ;
        double de_2 = u.de * u.de ;
        sigma1(0,0) = 1.0 / de_2 ;
        double az_2 = u.az * u.az ;
        sigma1(1,1) = 1.0 / az_2 ;
        double det1 = sigma1(0,0) * sigma1(1,1) ;

        for(std::vector<eigenverb>::iterator j=_bottom.begin();
                j!=_bottom.end(); ++j)
        {
                // Gather data for the return path from the bottom
            eigenverb v = (*j) ;
            double travel_time = u.time + v.time ;
                // Don't make contributions anymore if the travel time
                // is greater then the max reverberation curve time
            if( _max_time < travel_time ) break ;
            matrix<double> mu2(2,1) ;
            matrix<double> sigma2(2,2) ;
            mu2(0,0) = v.pos.latitude() ;
            mu2(1,0) = v.pos.longitude() ;
            sigma2(0,1) = sigma2(1,0) = 0.0 ;
            double de_2 = v.de * v.de ;
            sigma2(0,0) = 1.0 / de_2 ;
            double az_2 = v.az * v.az ;
            sigma2(1,1) = 1.0 / az_2 ;
            double det2 = sigma2(0,0) * sigma2(1,1) ;

            matrix<double> mu = mu1 - mu2 ;
            matrix<double> sigma = sigma1 + sigma2 ;

            double e = gaussian(mu,sigma) ;
            double tl_2way = v.intensity(0) * v.intensity(0) ;
            double s_sr = 1.0 ;         /// @todo add scatter_strength function
            unsigned t = floor( _num_bins * travel_time / _max_time ) ;
            _energy[t] = TWO_PI_2 * _pulse * tl_2way * s_sr
                            * det1 * det2 * e ;
        }
    }

}
