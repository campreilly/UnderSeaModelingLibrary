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
    double de1 ;
    double az1 ;
    eigenverb e1 = u ;
    e1.ndir.direction( &de1, &az1 ) ;
    double de2 ;
    double az2 ;
    eigenverb e2 = v ;
    e2.ndir.direction( &de2, &az2 ) ;
    double theta = abs( az2 - az1 ) ;
    #ifdef EIGENVERB_MODEL_DEBUG
        cout << "Contribution data:" << endl ;
        cout << "\ttravel_time: " << travel_time
                  << " theta: " << theta << endl ;
    #endif
            // Compute the intersection of the gaussian profiles
    double Wr_2 = v.sigma_az*v.sigma_az ;
    double Lr_2 = v.sigma_de*v.sigma_de ;
    double det_r = Wr_2 * Lr_2 ;
    double Ws_2 = u.sigma_az*u.sigma_az ;
    double Ls_2 = u.sigma_de*u.sigma_de ;
    double det_s = Ws_2 * Ls_2 ;
    double det_sr = 0.5 * ( 2.0*(Ls_2*Ws_2 + Lr_2*Wr_2)+(Ls_2+Ws_2)*(Lr_2+Wr_2) - (Ls_2-Ws_2)*(Lr_2-Wr_2)*cos(2.0*theta) ) ;
    double xs = abs(u.pos.longitude() - v.pos.longitude()) ;
    double ys = abs(u.pos.latitude() - v.pos.latitude()) ;
    double kappa = -0.25 * (xs*((Ls_2+Ws_2)+(Ls_2-Ws_2)*cos(2.0*theta)+2.0*Lr_2)
                          + ys*((Ls_2+Ws_2)-(Ls_2-Ws_2)*cos(2.0*theta)+2.0*Wr_2)
                          - xs*ys*(Ls_2-Ws_2)*sin(2.0*theta)) / det_sr ;
    double _area = sqrt(det_r) * sqrt(det_s) * exp( kappa ) / sqrt(det_sr) ;

            // Compute the energy reflected of of this patch
    vector<double> _loss( (*u.frequencies).size() ) ;
    boundary->reflect_loss( u.pos, *(u.frequencies), u.grazing, &_loss ) ;
    _loss = pow( 10.0, -0.05 * _loss ) ;
    vector<double> TL_in = element_prod( u.intensity, _loss ) ;
    vector<double> two_way_TL = element_prod( TL_in, v.intensity ) ;
        // calculate the scattering loss from the interface
    vector<double> s_sr( (*v.frequencies).size() ) ;
    boundary->getScattering_Model()->scattering_strength( v.pos, (*v.frequencies),
            u.grazing, v.grazing, u.az, v.az, &s_sr ) ;
    double _energy = TWO_PI * _pulse * two_way_TL(0) * s_sr(0) * _area ;

        // Only added value if contribution is significant
    if ( _energy > 1e-20 ) {
            // Calculate the time spread of the energy
        double det_sr_inv = det_sr / (det_r*det_s) ;
        double sigma_p_yy = 0.5 * ( (1.0/Ls_2+1.0/Ws_2)+(1.0/Ls_2-1.0/Ws_2)*cos(2.0*theta)+2.0/Wr_2 ) / det_sr_inv ;
        double Tarea = sqrt(sigma_p_yy)*cos(v.grazing) / v.c ;
        double Tsr = 0.5 * sqrt(_pulse*_pulse + Tarea*Tarea) ;
        double time = travel_time + Tsr ;
        vector<double> time_exp = (_two_way_time-time) * ( 1.0 / Tsr ) ;
        time_exp = element_prod( time_exp, time_exp ) ;
        vector<double> _time_spread =  exp( -0.5 * time_exp ) * ( 1.0 / ( Tsr * sqrt(TWO_PI) ) ) ;
        _time_spread *= _energy ;

        #ifdef EIGENVERB_MODEL_DEBUG
            cout << "\tcontribution: " << _time_spread << " bin: " << t << endl ;
        #endif

        _reverberation_curve += _time_spread ;
    }
}
