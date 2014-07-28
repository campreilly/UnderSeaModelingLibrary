/**
 * @file eigenverb_monostatic.cc
 */

#include <usml/waveq3d/eigenverb_monostatic.h>
#include <usml/waveq3d/spreading_hybrid_gaussian.h>

# define TWO_PI_2 (4.0*M_PI*M_PI)

//#define COLLISION_DEBUG
//#define MONOSTATIC_DEBUG

using namespace usml::waveq3d ;

/**  Constructor  **/
eigenverb_monostatic::eigenverb_monostatic( ocean_model& ocean,
    wave_queue_reverb& wave, double pulse, unsigned num_bins, double max_time ) :
    _pulse(pulse),
    _max_index(num_bins-1),
    _max_time(max_time),
    _reverberation_curve(num_bins)
{
    _spreading_model = wave.getSpreading_Model() ;
    _bottom_boundary = &ocean.bottom() ;
    _surface_boundary = &ocean.surface() ;
    _volume_boundary = ocean.volume() ;
    _bottom_loss.resize( wave.getFreqSize() ) ;
    _surface_loss.resize( wave.getFreqSize() ) ;
    _volume_loss.resize( wave.getFreqSize() ) ;
    unsigned _n = ocean.volume()->getNumberOfLayers() ;
    _upper.resize( _n ) ;
    _lower.resize( _n ) ;
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
               const wposition1& position, const wvector1& ndirection,
               const vector<double>& boundary_loss, unsigned ID )
{
    #ifdef COLLISION_DEBUG
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

    #ifdef COLLISION_DEBUG
        std::cout << "\t---Calling _spreading_model->getIntensity()---" << std::endl ;
        std::cout << "de: " << de << " az: " << az << std::endl ;
        std::cout << "offset: " << offset << " distance: " << distance << std::endl ;
    #endif

    const vector<double> amp = _spreading_model->getIntensity( position, de, az, offset, distance ) ;
    verb.intensity = element_prod( amp, boundary_loss ) ;
    verb.sigma_de = _spreading_model->getWidth_DE( de, az, offset ) ;
    verb.sigma_az = _spreading_model->getWidth_AZ( de, az, offset ) ;

    #ifdef COLLISION_DEBUG
        std::cout << "\t---Added eigenverb to _surface/_upper vector---" << endl ;
        std::cout << "\tverb de: " << verb.de << " az: " << verb.az
                  << " time: " << verb.time << std::endl ;
        std::cout << "\tgrazing: " << verb.grazing*180.0/M_PI << " speed: " << verb.c << std::endl ;
        std::cout << "\tintensity: " << verb.intensity << " sigma_de: " << verb.sigma_de
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
               const wposition1& position, const wvector1& ndirection,
               const vector<double>& boundary_loss, unsigned ID )
{
    #ifdef COLLISION_DEBUG
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

    #ifdef COLLISION_DEBUG
        std::cout << "\t---Calling _spreading_model->getIntensity---" << std::endl ;
        std::cout << "de: " << de << " az: " << az << std::endl ;
        std::cout << "offset: " << offset << " distance: " << distance << std::endl ;
    #endif

    const vector<double> amp = _spreading_model->getIntensity( position, de, az, offset, distance ) ;
    verb.intensity = element_prod( amp, boundary_loss ) ;
    verb.sigma_de = _spreading_model->getWidth_DE( de, az, offset ) ;
    verb.sigma_az = _spreading_model->getWidth_AZ( de, az, offset ) ;

    #ifdef COLLISION_DEBUG
        std::cout << "\t---Added eigenverb to _bottom/_lower vector---" << endl ;
        std::cout << "\tverb de: " << verb.de << " az: " << verb.az
                  << " time: " << verb.time << std::endl ;
        std::cout << "\tgrazing: " << verb.grazing*180.0/M_PI << " speed: " << verb.c << std::endl ;
        std::cout << "\tintensity: " << verb.intensity << " sigma_de: " << verb.sigma_de
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
    compute_contribution( _bottom, _bottom_boundary ) ;
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
    compute_contribution( _surface, _surface_boundary ) ;
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
    unsigned layer = 0 ;
    for(std::vector<std::vector<eigenverb> >::iterator k=_upper.begin();
            k!=_upper.end() && layer <= _n; ++k)
    {
        boundary_model* current_layer = _volume_boundary->getLayer(layer) ;
        compute_contribution( &k, current_layer) ;
        ++layer ;
    }
}

/** Compute all of the lower collision contributions**/
void eigenverb_monostatic::compute_lower_volume() {
    #ifdef MONOSTATIC_DEBUG
        std::cout << "Entering eigenverb_monostatic::compute_lower_volume()"
                  << std::endl ;
    #endif
    unsigned layer = 0 ;
    for(std::vector<std::vector<eigenverb> >::iterator k=_upper.begin();
            k!=_upper.end() && layer <= _n; ++k)
    {
        boundary_model* current_layer = _volume_boundary->getLayer(layer) ;
        compute_contribution( &k, current_layer) ;
        ++layer ;
    }
}

/****/
void eigenverb_monostatic::compute_contribution( const std::vector<eigenverb>& set,
                                                 const boundary_model* boundary )
{
    // Convolution of all paths on this boundary
    for(std::vector<eigenverb>::iterator i=set.begin();
            i!=set.end(); ++i)
    {
            // Gather data for the out path to the boundary
        eigenverb u = (*i) ;
        matrix<double> mu1 = mu(u) ;
        matrix<double> sigma1 = sigma(u) ;

        for(std::vector<eigenverb>::iterator j=set.begin();
                j!=set.end(); ++j)
        {
                // Gather data for the return path from the boundary
            eigenverb v = (*j) ;
    //            if ( u.az != v.az ) continue ;
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
            double _energy = energy( u, v, dA, boundary ) ;

                // Only added value if contribution is significant
            if ( _energy > 1e-20 ) {
                    // determine where on the temporally this contributions calls on the curve
                    // and add it to the cumulative curve value.
                unsigned t = floor( _max_index * travel_time / _max_time ) ;
                double two_way = _max_time * t / _max_index ;

                    // Calculate the time spread of the energy
                double _time_spread = time_spread( v, sigma1, sigma2, travel_time, two_way ) ;

                double value = _energy * _time_spread ;

                #ifdef MONOSTATIC_DEBUG
                    std::cout << "\tcontribution: " << value << " bin: " << t << std::endl ;
                #endif

                _reverberation_curve(t) += value ;
            }
        }
    }
}

/****/
inline matrix<double> eigenverb_monostatic::mu( const eigenverb& e ) {
    matrix<double> t (2,1) ;
    t(0,0) = e.pos.longitude() ;
    t(1,0) = e.pos.latitude() ;
    return t ;
}

/****/
inline matrix<double> eigenverb_monostatic::sigma( const eigenverb& e, double theta ) {
        // create a non-rotated matrix with covariances
    matrix<double> t (2,2) ;
    t(0,1) = t(1,0) = 0.0 ;
    t(0,0) = e.sigma_az * e.sigma_az ;
    t(1,1) = e.sigma_de * e.sigma_de / (sin(e.grazing) * sin(e.grazing)) ;
    if ( theta > 1e-20 ) {
            // create a rotation matrix
        matrix<double> r (2,2) ;
        r(0,0) = cos(theta) ;
        r(1,0) = sin(theta) ;
        r(0,1) = -sin(theta) ;
        r(1,1) = cos(theta) ;
        matrix<double> r_trans = trans(r) ;
            // apply rotation
        t = prod( r, t ) ;
        t = prod( t, r_trans ) ;
    }
    return t ;
}

/****/
inline double eigenverb_monostatic::area( const matrix<double>& mu1, const matrix<double>& sigma1,
                                          const matrix<double>& mu2, const matrix<double>& sigma2 )
{
    matrix<double> mu = mu1 - mu2 ;
    matrix<double> sigma = sigma1 + sigma2 ;
    double d1 = sqrt( sigma1(0,0)*sigma1(1,1) - sigma1(0,1)*sigma1(1,0) ) ;
    double d2 = sqrt( sigma2(0,0)*sigma2(1,1) - sigma2(0,1)*sigma2(1,0) ) ;

        // determinent and inverse of sigma
    double det = sigma(0,0)*sigma(1,1) - sigma(0,1)*sigma(1,0) ;
    matrix<double> s_inv(sigma) ;
    s_inv(0,0) = sigma(1,1) ;
    s_inv(1,1) = sigma(0,0) ;
    s_inv(0,1) = -sigma(0,1) ;
    s_inv(1,0) = -sigma(1,0) ;
    s_inv /= det ;
        // exponent product
    matrix<double> mu_trans = trans(mu) ;
    matrix<double> mu_prod = prod( mu_trans, s_inv ) ;
    matrix<double> kappa = prod( mu_prod, mu ) ;
    det = 1.0 / sqrt(det) ;
    kappa *= -0.5 ;

        // calculate the area
    double a = 0.5 * d1 * d2 * det * exp( kappa(0,0) ) ;
    #ifdef MONOSTATIC_DEBUG
        std::cout << "\tmu: " << mu << " sigma: " << sigma << std::endl ;
        std::cout << "\tarea: " << a << std::endl ;
    #endif
    return  a ;
}

/****/
inline double eigenverb_monostatic::energy( const eigenverb& in, const eigenverb& out,
                                            const double dA, boundary_model* b )
{
        // calculate the additional loss due to collision with the boundary
        // for only one direction of the transmission
    vector<double> _loss( (*in.frequencies).size() ) ;
    b->reflect_loss( in.pos, *(in.frequencies), in.grazing, &_loss ) ;
    for(unsigned f=0; f<(*in.frequencies).size(); ++f) {
        _loss(f) = pow( 10.0, _loss(f) / -20.0 ) ;
    }
    vector<double> TL_in = element_prod( in.intensity, _loss ) ;
    vector<double> two_way_TL = element_prod( TL_in, out.intensity ) ;
        // calculate the scattering loss from the interface
    vector<double> s_sr( (*out.frequencies).size() ) ;
    vector<double> phase( (*out.frequencies).size() ) ;
    b->getScattering_Model()->scattering_strength( out.pos, (*out.frequencies),
            in.grazing, out.grazing, in.az, out.az, &s_sr, &phase ) ;
        // caluclate the total energy reflected from this patch
    double _e = _pulse * two_way_TL(0) * s_sr(0) * dA ;
    #ifdef MONOSTATIC_DEBUG
        std::cout << "\tbottom loss: " << _loss
                  << " scattering strength: " << s_sr
                  << std::endl ;
        std::cout << "\tTL to patch: " << in.intensity
                  << " TL from patch: " << out.intensity
                  << std::endl ;
        std::cout << "\tenergy: " << _e << std::endl ;
    #endif
    return _e ;
}

/****/
inline double eigenverb_monostatic::time_spread( const eigenverb& out, const matrix<double>& s1,
                                                 const matrix<double>& s2, const double travel_time,
                                                 const double two_way_time )
{
    double time = travel_time ;
    double l_r = 1.0 / s1(1,1) ;
    double l_s = 1.0 / s2(1,1) ;
    double sigma_p = sqrt( 1.0 / (l_r + l_s) ) ;
    double Tarea = sigma_p * sin(out.grazing) / out.c ;
    double T_sr = sqrt( _pulse*_pulse + Tarea*Tarea ) / 2.0 ;               /// @caution should this be sine or cosine of grazing angle????
    time += T_sr ;
    double time_exp = (two_way_time-time) / T_sr ;
    #ifdef MONOSTATIC_DEBUG
        std::cout << "\tT_sr: " << T_sr << std::endl ;
    #endif
    return exp( -0.5 * time_exp * time_exp ) / ( T_sr * sqrt(TWO_PI) ) ;
}
