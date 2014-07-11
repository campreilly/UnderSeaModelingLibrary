/**
 * @file eigenverb_monostatic.cc
 */

#include <usml/waveq3d/eigenverb_monostatic.h>
#include <usml/waveq3d/spreading_hybrid_gaussian.h>

# define TWO_PI_2 (4.0*M_PI*M_PI)

using namespace usml::waveq3d ;

/**  Constructor  **/
eigenverb_monostatic::eigenverb_monostatic( ocean_model& ocean,
    wave_queue_reverb& wave, double pulse, unsigned num_bins, double max_time ) :
    _pulse(pulse),
    _num_bins(num_bins),
    _max_time(max_time)
{
    _spreading_model = wave.getSpreading_Model() ;
    _bottom_scatter = ocean.bottom().getScattering_Model() ;
    _surface_scatter = ocean.surface().getScattering_Model() ;
    _volume_scatter = ocean.volume()->getScattering_Model() ;
    unsigned n = ocean.volume()->getNumberOfLayers() ;
    _upper.resize(n) ;
    _lower.resize(n) ;
    _origin = wave.getOrigin() ;
    _energy = new double[_num_bins] ;
    memset(_energy,0.0,_num_bins) ;
}

/**  Destructor  **/
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
               const wposition1& position, const wvector1& ndirection, unsigned ID ) {

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
        case 10:
            _surface.push_back( verb ) ;
            break ;
        default:
            ID -= _origin ;
            _upper.at(ID).push_back( verb ) ;
            break ;
    }
}

/**
 * Places an eigenverb into the class of "lower" bins to be used for the overall
 * reverberation calculation.
 */
void eigenverb_monostatic::notifyLowerCollision( unsigned de, unsigned az, double time,
               double dt, double grazing, double speed, const seq_vector& frequencies,
               const wposition1& position, const wvector1& ndirection, unsigned ID ) {

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
        case 10:
            _bottom.push_back( verb ) ;
            break ;
        default:
            ID -= _origin ;
            _lower.at(ID).push_back( verb ) ;
            break ;
    }
}

/**
 * Computes the reverberation curve from the data cataloged from the
 * wavefront(s).
 */
void eigenverb_monostatic::compute_reverberation() {
        // Contributions from the bottom collisions
    compute_bottom_energy() ;
        // Contributions from the surface collisions
    compute_surface_energy() ;
        // Contributions from the volume layers
    compute_volume_energy() ;
}

/**
 * Computes the energy contributions to the reverberation
 * energy curve from the bottom interactions.
 */
void eigenverb_monostatic::compute_bottom_energy() {
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
            vector<double> s_sr ;
            vector<double> phase ;
            _bottom_scatter->scattering_strength( v.pos,
                (*v.frequencies), u.grazing, v.grazing, u.az, v.az, &s_sr, &phase ) ;
            unsigned t = floor( _num_bins * travel_time / _max_time ) ;
            _energy[t] += TWO_PI_2 * _pulse * tl_2way * s_sr(0)      /// @todo _energy is not a vector of freq
                            * det1 * det2 * e ;
        }
    }

}

/**
 * Computes the energy contributions to the reverberation
 * energy curve from the surface interactions.
 */
void eigenverb_monostatic::compute_surface_energy() {
    // Convolution of all surface paths
    for(std::vector<eigenverb>::iterator i=_surface.begin();
            i!=_surface.end(); ++i)
    {
            // Gather data for the out path to the surface
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

        for(std::vector<eigenverb>::iterator j=_surface.begin();
                j!=_surface.end(); ++j)
        {
                // Gather data for the return path from the surface
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
            vector<double> s_sr ;
            vector<double> phase ;
            _surface_scatter->scattering_strength( v.pos,
                (*v.frequencies), u.grazing, v.grazing, u.az, v.az, &s_sr, &phase ) ;
            unsigned t = floor( _num_bins * travel_time / _max_time ) ;
            _energy[t] += TWO_PI_2 * _pulse * tl_2way * s_sr(0)      /// @todo _energy is not a vector of freq
                            * det1 * det2 * e ;
        }
    }
}

/**
 * Computes the energy contributions to the reverberation
 * energy curve from the volume interactions.
 */
void eigenverb_monostatic::compute_volume_energy() {
        // Contributions from the upper volume layer collisions
    compute_upper_volume() ;
        // Contributions from the lower volume layer collisions
    compute_lower_volume() ;
}

/** Compute all of the upper collision contributions**/
void eigenverb_monostatic::compute_upper_volume() {
    for(std::vector<std::vector<eigenverb> >::iterator k=_upper.begin();
            k!=_upper.end(); ++k)
    {
        for(std::vector<eigenverb>::iterator i=k->begin();
                i!=k->end(); ++i)
        {
                // Gather data for the out path to the upper collision
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

            for(std::vector<eigenverb>::iterator j=k->begin();
                    j!=k->end(); ++j)
            {
                    // Gather data for the return path from the upper collision
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
                vector<double> s_sr ;
                vector<double> phase ;
                _surface_scatter->scattering_strength( v.pos,
                    (*v.frequencies), u.grazing, v.grazing, u.az, v.az, &s_sr, &phase ) ;
                unsigned t = floor( _num_bins * travel_time / _max_time ) ;
                _energy[t] += TWO_PI_2 * _pulse * tl_2way * s_sr(0)      /// @todo _energy is not a vector of freq
                                * det1 * det2 * e ;
            }
        }
    }
}

/** Compute all of the lower collision contributions**/
void eigenverb_monostatic::compute_lower_volume() {
    for(std::vector<std::vector<eigenverb> >::iterator k=_lower.begin();
            k!=_lower.end(); ++k)
    {
        for(std::vector<eigenverb>::iterator i=k->begin();
                i!=k->end(); ++i)
        {
                // Gather data for the out path to the lower collision
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

            for(std::vector<eigenverb>::iterator j=k->begin();
                    j!=k->end(); ++j)
            {
                    // Gather data for the return path from the lower collision
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
                vector<double> s_sr ;
                vector<double> phase ;
                _surface_scatter->scattering_strength( v.pos,
                    (*v.frequencies), u.grazing, v.grazing, u.az, v.az, &s_sr, &phase ) ;
                unsigned t = floor( _num_bins * travel_time / _max_time ) ;
                _energy[t] += TWO_PI_2 * _pulse * tl_2way * s_sr(0)      /// @todo _energy is not a vector of freq
                                * det1 * det2 * e ;
            }
        }
    }
}
