/**
 * @file eigenverb_monostatic.cc
 */

#include <usml/waveq3d/eigenverb_monostatic.h>
#include <usml/waveq3d/spreading_hybrid_gaussian.h>

# define TWO_PI_2 (4.0*M_PI*M_PI)

//#define MONOSTATIC_DEBUG

using namespace usml::waveq3d ;

/**  Constructor  **/
eigenverb_monostatic::eigenverb_monostatic( ocean_model& ocean,
    wave_queue_reverb& wave, double pulse, unsigned num_bins, double max_time ) :
    _pulse(pulse),
    _num_bins(num_bins-1),
    _max_time(max_time),
    _reverberation_curve(num_bins)
{
    _spreading_model = wave.getSpreading_Model() ;
    _bottom_scatter = ocean.bottom().getScattering_Model() ;
    _surface_scatter = ocean.surface().getScattering_Model() ;
    _volume_scatter = ocean.volume()->getScattering_Model() ;
    unsigned n = ocean.volume()->getNumberOfLayers() ;
    _upper.resize(n) ;
    _lower.resize(n) ;
    _origin = wave.getID() ;
    for(vector<double>::iterator i=_reverberation_curve.begin();
            i!=_reverberation_curve.end(); ++i) {
        (*i) = 1e-20 ;
    }
}

/**
 * Places an eigenverb into the class of "upper" bins to be used for the overall
 * reverberation calculation.
 */
void eigenverb_monostatic::notifyUpperCollision( unsigned de, unsigned az, double time,
               double dt, double grazing, double speed, const seq_vector& frequencies,
               const wposition1& position, const wvector1& ndirection, unsigned ID )
{
    #ifdef MONOSTATIC_DEBUG
        std::cout << "**** Entering eigenverb_monostatic::notifyUpperCollision" << std::endl ;
        std::cout << "de: " << de << " az: " << az << " time: " << time << std::endl ;
        std::cout << "grazing: " << grazing << " ID: " << ID << std::endl ;
    #endif

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

    #ifdef MONOSTATIC_DEBUG
        std::cout << "\t---Calling _spreading_model->getIntensity()---" << std::endl ;
        std::cout << "de: " << de << " az: " << az << std::endl ;
        std::cout << "offset: " << offset << " distance: " << distance << std::endl ;
    #endif

    const vector<double> amp = _spreading_model->getIntensity( position, de, az, offset, distance ) ;
    verb.intensity = - 10.0 * log10( amp ) ;
    verb.sigma_de = _spreading_model->getWidth_DE( de, az, offset ) ;
    verb.sigma_az = _spreading_model->getWidth_AZ( de, az, offset ) ;

    #ifdef MONOSTATIC_DEBUG
        std::cout << "\t---Added eigenverb to _surface/_upper vector---" << endl ;
        std::cout << "\tverb de: " << verb.de << " az: " << verb.az
                  << " time: " << verb.time << std::endl ;
        std::cout << "\tgrazing: " << verb.grazing << " speed: " << verb.c << std::endl ;
        std::cout << "\tintensity: " << verb.intensity(0) << " sigma_de: " << verb.sigma_de
                  << " sigma_az: " << verb.sigma_az << std::endl ;
    #endif
    switch (ID) {
        case 10:
            _surface.push_back( verb ) ;
            break ;
        default:
            ID -= _origin - 1 ;
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
               const wposition1& position, const wvector1& ndirection, unsigned ID )
{
    #ifdef MONOSTATIC_DEBUG
        std::cout << "**** Entering eigenverb_monostatic::notifyLowerCollision" << std::endl ;
        std::cout << "de: " << de << " az: " << az << " time: " << time << std::endl ;
        std::cout << "grazing: " << grazing << " ID: " << ID << std::endl ;
    #endif

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

    #ifdef MONOSTATIC_DEBUG
        std::cout << "\t---Calling _spreading_model->getIntensity---" << std::endl ;
        std::cout << "de: " << de << " az: " << az << std::endl ;
        std::cout << "offset: " << offset << " distance: " << distance << std::endl ;
    #endif

    const vector<double> amp = _spreading_model->getIntensity( position, de, az, offset, distance ) ;
    verb.intensity = amp ;
    verb.sigma_de = _spreading_model->getWidth_DE( de, az, offset ) ;
    verb.sigma_az = _spreading_model->getWidth_AZ( de, az, offset ) ;

    #ifdef MONOSTATIC_DEBUG
        std::cout << "\t---Added eigenverb to _bottom/_lower vector---" << endl ;
        std::cout << "\tverb de: " << verb.de << " az: " << verb.az
                  << " time: " << verb.time << std::endl ;
        std::cout << "\tgrazing: " << verb.grazing << " speed: " << verb.c << std::endl ;
        std::cout << "\tintensity: " << verb.intensity(0) << " sigma_de: " << verb.sigma_de
                  << " sigma_az: " << verb.sigma_az << std::endl ;
    #endif
    switch (ID) {
        case 10:
            _bottom.push_back( verb ) ;
            break ;
        default:
            ID -= _origin - 1 ;
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
    #ifdef MONOSTATIC_DEBUG
        std::cout << "Entering eigenverb_monostatic::compute_bottom_energy()"
                  << std::endl ;
    #endif
    // Convolution of all bottom paths
    for(std::vector<eigenverb>::iterator i=_bottom.begin();
            i!=_bottom.end(); ++i)
    {
            // Gather data for the out path to the bottom
        eigenverb u = (*i) ;
        matrix<double> mu1 = mu(u) ;
        matrix<double> sigma1 = sigma(u) ;

        for(std::vector<eigenverb>::iterator j=_bottom.begin();
                j!=_bottom.end(); ++j)
        {
                // Gather data for the return path from the bottom
            eigenverb v = (*j) ;
            double travel_time = u.time + v.time ;
                // Don't make contributions anymore if the travel time
                // is greater then the max reverberation curve time
            if( _max_time <= travel_time ) break ;
            #ifdef MONOSTATIC_DEBUG
                std::cout << "Contribution data:" << std::endl ;
                std::cout << "\ttravel_time: " << travel_time << std::endl ;
            #endif
            matrix<double> mu2 = mu(v) ;
            matrix<double> sigma2 = sigma(v) ;
            double dA = area( mu1, sigma1, mu2, sigma2 ) ;

                // Compute the energy reflected of of this patch
            double _energy = energy( u, v, dA, _bottom_scatter ) ;

                // Calculate the time spread of the energy
            double _time_spread = time_spread( v, sigma1, sigma2, travel_time ) ;

                // Only added value if contribution is significant
            double value = _energy * _time_spread ;
            if ( value > 1e-20 ) {
                    // determine where on the temporally this contributions calls on the curve
                    // and add it to the cumulative curve value.
                unsigned t = floor( _num_bins * travel_time / _max_time ) ;

                #ifdef MONOSTATIC_DEBUG
                    std::cout << "\tcontribution: " << value << " bin: " << t << std::endl ;
                #endif

                _reverberation_curve(t) += value ;
            }
        }
    }

}

/**
 * Computes the energy contributions to the reverberation
 * energy curve from the surface interactions.
 */
void eigenverb_monostatic::compute_surface_energy() {
    #ifdef MONOSTATIC_DEBUG
        std::cout << "Entering eigenverb_monostatic::compute_surface_energy()"
                  << std::endl ;
    #endif
    // Convolution of all surface paths
    for(std::vector<eigenverb>::iterator i=_surface.begin();
            i!=_surface.end(); ++i)
    {
            // Gather data for the out path to the surface
        eigenverb u = (*i) ;
        matrix<double> mu1 = mu(u) ;
        matrix<double> sigma1 = sigma(u) ;

        for(std::vector<eigenverb>::iterator j=_surface.begin();
                j!=_surface.end(); ++j)
        {
                // Gather data for the return path from the surface
            eigenverb v = (*j) ;
            double travel_time = u.time + v.time ;
                // Don't make contributions anymore if the travel time
                // is greater then the max reverberation curve time
            if( _max_time <= travel_time ) break ;
            #ifdef MONOSTATIC_DEBUG
                std::cout << "Contribution data:" << std::endl ;
                std::cout << "\ttravel_time: " << travel_time << std::endl ;
            #endif
            matrix<double> mu2 = mu(v) ;
            matrix<double> sigma2 = sigma(v) ;
            double dA = area( mu1, sigma1, mu2, sigma2 ) ;

                // Compute the energy reflected of of this patch
            double _energy = energy( u, v, dA, _surface_scatter ) ;

                // Calculate the time spread of the energy
            double _time_spread = time_spread( v, sigma1, sigma2, travel_time ) ;

                // Only added value if contribution is significant
            double value = _energy * _time_spread ;
            if ( value > 1e-20 ) {
                    // determine where on the temporally this contributions calls on the curve
                    // and add it to the cumulative curve value.
                unsigned t = floor( _num_bins * travel_time / _max_time ) ;

                #ifdef MONOSTATIC_DEBUG
                    std::cout << "\tcontribution: " << value << " bin: " << t << std::endl ;
                #endif

                _reverberation_curve(t) += value ;
            }
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
    #ifdef MONOSTATIC_DEBUG
        std::cout << "Entering eigenverb_monostatic::compute_upper_volume()"
                  << std::endl ;
    #endif
    for(std::vector<std::vector<eigenverb> >::iterator k=_upper.begin();
            k!=_upper.end(); ++k)
    {
        for(std::vector<eigenverb>::iterator i=k->begin();
                i!=k->end(); ++i)
        {
            // Gather data for the out path to the upper collision
            eigenverb u = (*i) ;
            matrix<double> mu1 = mu(u) ;
            matrix<double> sigma1 = sigma(u) ;

            for(std::vector<eigenverb>::iterator j=k->begin();
                    j!=k->end(); ++j)
            {
                    // Gather data for the return path from the upper collision
                eigenverb v = (*j) ;
                double travel_time = u.time + v.time ;
                    // Don't make contributions anymore if the travel time
                    // is greater then the max reverberation curve time
                if( _max_time <= travel_time ) break ;
                #ifdef MONOSTATIC_DEBUG
                    std::cout << "Contribution data:" << std::endl ;
                    std::cout << "\ttravel_time: " << travel_time << std::endl ;
                #endif
                matrix<double> mu2 = mu(v) ;
                matrix<double> sigma2 = sigma(v) ;
                double dA = area( mu1, sigma1, mu2, sigma2 ) ;

                    // Compute the energy reflected of of this patch
                double _energy = energy( u, v, dA, _volume_scatter ) ;

                    // Calculate the time spread of the energy
                double _time_spread = time_spread( v, sigma1, sigma2, travel_time ) ;

                    // Only added value if contribution is significant
                double value = _energy * _time_spread ;
                if ( value > 1e-20 ) {
                        // determine where on the temporally this contributions calls on the curve
                        // and add it to the cumulative curve value.
                    unsigned t = floor( _num_bins * travel_time / _max_time ) ;

                    #ifdef MONOSTATIC_DEBUG
                        std::cout << "\tcontribution: " << value << " bin: " << t << std::endl ;
                    #endif

                    _reverberation_curve(t) += value ;
                }
            }
        }
    }
}

/** Compute all of the lower collision contributions**/
void eigenverb_monostatic::compute_lower_volume() {
    #ifdef MONOSTATIC_DEBUG
        std::cout << "Entering eigenverb_monostatic::compute_lower_volume()"
                  << std::endl ;
    #endif
    for(std::vector<std::vector<eigenverb> >::iterator k=_lower.begin();
            k!=_lower.end(); ++k)
    {
        for(std::vector<eigenverb>::iterator i=k->begin();
                i!=k->end(); ++i)
        {
            // Gather data for the out path to the lower collision
            eigenverb u = (*i) ;
            matrix<double> mu1 = mu(u) ;
            matrix<double> sigma1 = sigma(u) ;

            for(std::vector<eigenverb>::iterator j=k->begin();
                    j!=k->end(); ++j)
            {
                    // Gather data for the return path from the lower collision
                eigenverb v = (*j) ;
                double travel_time = u.time + v.time ;
                    // Don't make contributions anymore if the travel time
                    // is greater then the max reverberation curve time
                if( _max_time <= travel_time ) break ;
                #ifdef MONOSTATIC_DEBUG
                    std::cout << "Contribution data:" << std::endl ;
                    std::cout << "\ttravel_time: " << travel_time << std::endl ;
                #endif
                matrix<double> mu2 = mu(v) ;
                matrix<double> sigma2 = sigma(v) ;
                double dA = area( mu1, sigma1, mu2, sigma2 ) ;

                    // Compute the energy reflected of of this patch
                double _energy = energy( u, v, dA, _volume_scatter ) ;

                    // Calculate the time spread of the energy
                double _time_spread = time_spread( v, sigma1, sigma2, travel_time ) ;

                    // Only added value if contribution is significant
                double value = _energy * _time_spread ;
                if ( value > 1e-20 ) {
                        // determine where on the temporally this contributions calls on the curve
                        // and add it to the cumulative curve value.
                    unsigned t = floor( _num_bins * travel_time / _max_time ) ;

                    #ifdef MONOSTATIC_DEBUG
                        std::cout << "\tcontribution: " << value << " bin: " << t << std::endl ;
                    #endif

                    _reverberation_curve(t) += value ;
                }
            }
        }
    }
}

/****/
inline matrix<double> eigenverb_monostatic::mu( const eigenverb& e ) {
    matrix<double> t (2,1) ;
    t(0,0) = e.pos.latitude() ;
    t(1,0) = e.pos.longitude() ;
    return t ;
}

/** Sigma inverse **/
inline matrix<double> eigenverb_monostatic::sigma( const eigenverb& e ) {
    matrix<double> t (2,2) ;
    t(0,1) = t(1,0) = 0.0 ;
    t(0,0) = e.sigma_de * e.sigma_de / (sin(e.grazing) * sin(e.grazing)) ;
    t(1,1) = e.sigma_az * e.sigma_az ;
    return t ;
}

/****/
inline double eigenverb_monostatic::area( const matrix<double>& mu1, const matrix<double>& sigma1,
                                          const matrix<double>& mu2, const matrix<double>& sigma2 )
{
    matrix<double> mu = mu1 - mu2 ;
    matrix<double> s = sigma1 + sigma2 ;
    double d1 = determinent(sigma1) ;
    double d2 = determinent(sigma2) ;
    double a = sqrt( d1 * d2 ) * gaussian(mu,s) ;
    #ifdef MONOSTATIC_DEBUG
        std::cout << "\tmu: " << mu << " sigma: " << s << std::endl ;
        std::cout << "\tarea: " << a << std::endl ;
    #endif
    return  a ;
}

/****/
inline double eigenverb_monostatic::energy( const eigenverb& in, const eigenverb& out,
                                            const double dA, scattering_model* s )
{
    vector<double> s_sr( (*out.frequencies).size() ) ;
    vector<double> phase( (*out.frequencies).size() ) ;
    s->scattering_strength( out.pos, (*out.frequencies), in.grazing, out.grazing,
                            in.az, out.az, &s_sr, &phase ) ;
    double _e = 0.5 * TWO_PI * _pulse * in.intensity(0) * out.intensity(0) * s_sr(0) * dA ;
    #ifdef MONOSTATIC_DEBUG
        std::cout << "\tenergy: " << _e ;
    #endif
    return _e ;
}

/****/
inline double eigenverb_monostatic::time_spread( const eigenverb& out, const matrix<double>& s1,
                                                 const matrix<double>& s2, const double travel_time )
{
    double time = travel_time ;
    double l_r = 1.0 / s1(0,0) ;
    double l_s = 1.0 / s2(0,0) ;
    double sigma_p = sqrt( 1.0 / (l_r + l_s) ) ;
    double T_sr = 0.25 * ( _pulse + sigma_p * sin(out.grazing) / out.c ) ;               /// @caution should this be sine or cosine of grazing angle????
    time += ( T_sr / 2.0 ) ;
//    time += T_sr ;
    double time_exp = (out.time-time) / T_sr ;
    #ifdef MONOSTATIC_DEBUG
        std::cout << " T_sr: " << T_sr << std::endl ;
    #endif
    return exp( -0.5 * time_exp * time_exp ) / ( T_sr * sqrt(TWO_PI) ) ;
}
