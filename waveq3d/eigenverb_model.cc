/**
 * @file eigenverb_model.h
 */

#include <usml/waveq3d/eigenverb_model.h>

using namespace usml::waveq3d ;

/**
 * Creates an eigenverb from the provided data
 */
void eigenverb_model::create_eigenverb( unsigned de, unsigned az, double time,
       double dt, double grazing, double speed, const seq_vector& frequencies,
       const wposition1& position, const wvector1& ndirection,
       const vector<double>& boundary_loss, eigenverb& verb ) const
{
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

    #ifdef EIGENVERB_COLLISION_DEBUG
        cout << "\t---Calling _spreading_model->getIntensity()---" << endl ;
        cout << "de: " << de << " az: " << az << endl ;
        cout << "offset: " << offset << " distance: " << distance << endl ;
    #endif

    const vector<double> amp = _spreading_model->getIntensity( position, de, az, offset, distance ) ;
    verb.intensity = element_prod( amp, boundary_loss ) ;
    verb.sigma_de = _spreading_model->getWidth_DE( de, az, offset ) ;
    verb.sigma_az = _spreading_model->getWidth_AZ( de, az, offset ) ;

    #ifdef EIGENVERB_COLLISION_DEBUG
        cout << "\t---Added eigenverb to collection---" << endl ;
        cout << "\tverb de: " << verb.de << " az: " << verb.az
             << " time: " << verb.time << endl ;
        cout << "\tgrazing: " << verb.grazing*180.0/M_PI << " speed: " << verb.c << endl ;
        cout << "\tintensity: " << verb.intensity << " sigma_de: " << verb.sigma_de
             << " sigma_az: " << verb.sigma_az << endl ;
    #endif
}

/**
 * Computes the contribution value
 */
void eigenverb_model::compute_contribution( const eigenverb& u, const eigenverb& v,
                                            boundary_model* boundary )
{
    double travel_time = u.time + v.time ;
    double de1, double az1 ;
    u.ndir.direction( &de1, &az1 ) ;
    double de2, double az2 ;
    v.ndir.direction( &de2, &az2 ) ;
    double theta = abs( az2 - az1 ) ;
    #ifdef EIGENVERB_MODEL_DEBUG
        cout << "Contribution data:" << endl ;
        cout << "\ttravel_time: " << travel_time
                  << " theta: " << theta << endl ;
    #endif
    matrix<double> mu1 = mu( u ) ;
    matrix<double> sigma1 = sigma( u, theta ) ;
    matrix<double> mu2 = mu( v ) ;
    matrix<double> sigma2 = sigma( v ) ;
        // Compute the intersection of the gaussian profiles
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

        #ifdef EIGENVERB_MODEL_DEBUG
            cout << "\tcontribution: " << value << " bin: " << t << endl ;
        #endif

        _reverberation_curve(t) += value ;
    }
}

/**
 * Creates a 2x1 matrix from the eigenverb data
 */
inline matrix<double> eigenverb_model::mu( const eigenverb& e ) {
    matrix<double> t (2,1) ;
    t(0,0) = e.pos.longitude() ;
    t(1,0) = e.pos.latitude() ;
    return t ;
}

/**
 * Gaussian profile for this eigenverb
 */
inline matrix<double> eigenverb_model::sigma( const eigenverb& e, double theta )
{
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

/**
 * Compute the intersection of the two gaussian profiles
 */
inline double eigenverb_model::area(
        const matrix<double>& mu1, const matrix<double>& sigma1,
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
    #ifdef EIGENVERB_MODEL_DEBUG
        cout << "\tmu: " << mu << " sigma: " << sigma << endl ;
        cout << "\tarea: " << a << endl ;
    #endif
    return  a ;
}

/**
 * Compute the energy reflected from this patch
 */
inline double eigenverb_model::energy( const eigenverb& in,
        const eigenverb& out, const double dA, boundary_model* b )
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
    #ifdef EIGENVERB_MODEL_DEBUG
        cout << "\tboundary loss: " << _loss
             << " scattering strength: " << s_sr
             << endl ;
        cout << "\tTL to patch: " << in.intensity
             << " TL from patch: " << out.intensity
             << endl ;
        cout << "\tenergy: " << _e << endl ;
    #endif
    return _e ;
}

/**
 * Spread the energy back out over time
 */
inline double eigenverb_model::time_spread(
        const eigenverb& out, const matrix<double>& s1,
        const matrix<double>& s2, const double travel_time,
        const double two_way_time )
{
    double time = travel_time ;
    matrix<double> s1_inv( s1 ) ;
    matrix<double> s2_inv( s2 ) ;
    inverse( s1, s1_inv ) ;
    inverse( s2, s2_inv ) ;
    matrix<double> sigma_p( s1 ) ;
    inverse( s1_inv + s2_inv, sigma_p ) ;
    double Tarea = sigma_p(1,1) * sin(out.grazing) / out.c ;
    double T_sr = sqrt( _pulse*_pulse + Tarea*Tarea ) / 2.0 ;               /// @caution should this be sine or cosine of grazing angle????
    time += T_sr ;
    double time_exp = (two_way_time-time) / T_sr ;
    #ifdef EIGENVERB_MODEL_DEBUG
        cout << "\tT_sr: " << T_sr << endl ;
    #endif
    return exp( -0.5 * time_exp * time_exp ) / ( T_sr * sqrt(TWO_PI) ) ;
}

/**
 * Produces a gaussian profile
 */
inline double eigenverb_model::gaussian( const matrix<double>& mu,
                                         const matrix<double>& sigma )
{
    matrix<double> s_inv(sigma) ;
    inverse( sigma, s_inv ) ;
    double det = determinent(TWO_PI*sigma) ;
    matrix<double> mu_trans = trans(mu) ;
    matrix<double> mu_prod = prod( mu_trans, s_inv ) ;
    matrix<double> kappa = prod( mu_prod, mu ) ;
    det = 1.0 / sqrt(det) ;
    kappa *= -0.5 ;
    return det * exp( kappa(0,0) ) ;
}

/**
 * Calculates the matrix determinent of a matrix
 */
inline double eigenverb_model::determinent( const matrix<double>& m ) {
    double det = 1.0 ;
    matrix<double> a(m) ;
    permutation_matrix<size_t> pivot( a.size1() ) ;
    if( lu_factorize(a,pivot) ) return 0.0 ;
    for(size_t i=0; i<pivot.size(); ++i) {
        if (pivot(i) != i) det *= -1.0 ;
        det *= a(i,i) ;
    }
    return det ;
}

/**
 * Computes the inverse of a matrix.
 */
inline bool eigenverb_model::inverse( const matrix<double>& m,
                                      matrix<double>& i )
{
    matrix<double> a(m) ;
    permutation_matrix<size_t> pm(a.size1()) ;
    int result = lu_factorize(a, pm) ;
    if( result != 0 ) return false ;
    i.assign(identity_matrix<double>(a.size1())) ;
    lu_substitute(a, pm, i) ;
    return true ;
}

