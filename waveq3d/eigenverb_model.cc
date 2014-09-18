/**
 * @file eigenverb_model.h
 */

#include <usml/waveq3d/eigenverb_model.h>

#define DEBUG_CONVOLUTION

using namespace usml::waveq3d ;

/**
 * Creates an eigenverb from the provided data
 */
void eigenverb_model::create_eigenverb( unsigned de, unsigned az,
               double dt, double grazing, double speed,
               const wposition1& position, const wvector1& ndirection,
               const wave_queue& wave, eigenverb& verb ) const
{
    verb.de_index = de ;
    verb.az_index = az ;
    verb.launch_az = wave.source_az(az) ;
    verb.launch_de = wave.source_de(de) ;
    verb.distance = wave.curr()->getPath_length(de,az) + speed * dt ;
    verb.time = wave.time() + dt ;
    verb.grazing = grazing ;
    verb.c = speed ;
    verb.pos = position ;
    verb.ndir = ndirection ;
    verb.frequencies = wave.frequencies() ;

        // Calculate the one way TL and the width of the gaussian
        // at the time of impact with the boundary.
    double true_distance = verb.distance ;
    double spreading_loss = 1.0 / (true_distance * true_distance) ;
    vector<double> amp( wave.frequencies()->size(), spreading_loss ) ;
    vector<double> boundary_loss = pow( 10.0, -0.05 * wave.curr()->getAttenuation(de,az) ) ;
    verb.intensity = element_prod( amp, boundary_loss ) ;

    double mu_half1 = M_PI * ( wave.source_de(de-1) + wave.source_de(de) ) / 360.0 ;
    double mu_half2 = M_PI * ( wave.source_de(de) + wave.source_de(de+1) ) / 360.0 ;
    double delta_de = mu_half2 - mu_half1 ;
    verb.sigma_de = true_distance * delta_de / sin(grazing) ;
//    double phi_half1 = M_PI * ( wave.source_az(az-1) + wave.source_az(az) ) / 360.0 ;
//    double phi_half2 = M_PI * ( wave.source_az(az) + wave.source_az(az+1) ) / 360.0 ;
//    double delta_az = phi_half2 - phi_half1 ;
//    verb.sigma_az = delta_az * cos(grazing) * true_distance ;   // horizontal distance * azimuthal spacing
    verb.sigma_az = TWO_PI * cos(grazing) * true_distance ;   // horizontal distance * azimuthal spacing

    #ifdef EIGENVERB_COLLISION_DEBUG
        cout << "\t---Added eigenverb to collection---" << endl ;
        cout << "\tverb de: " << verb.launch_de << " az: " << verb.launch_az
             << " time: " << verb.time << endl ;
        cout << "\tgrazing: " << verb.grazing*180.0/M_PI << " speed: " << verb.c << endl ;
        cout << "\tintensity: " << verb.intensity << " sigma_de: " << verb.sigma_de
             << " sigma_az: " << verb.sigma_az << endl ;
    #endif
}

/**
 * Computes the contribution value
 */
void eigenverb_model::compute_contribution( const eigenverb* u, const eigenverb* v,
                                            boundary_model* boundary )
{
        // determine the relative angle of the projected incident gaussian to the
        // projected reflected gaussian.
    double de1 ;
    double az1 ;
    u->ndir.direction( &de1, &az1 ) ;
    double de2 ;
    double az2 ;
    v->ndir.direction( &de2, &az2 ) ;
//    double alpha = abs( az2 - az1 ) ;
    double alpha = 0.0 ;

        // Compute the intersection of the gaussian profiles
    double Wr_2 = v->sigma_az*v->sigma_az ;
    double Lr = v->sigma_de ;
    double Lr_2 = Lr*Lr ;
    double det_r = Wr_2 * Lr_2 ;
    double Ws_2 = u->sigma_az*u->sigma_az ;
    double Ls = u->sigma_de ;
    double Ls_2 = Ls*Ls ;
    double det_s = Ws_2 * Ls_2 ;
    double det_sr = 0.5 * ( 2.0*(Ls_2*Ws_2 + Lr_2*Wr_2) + (Ls_2+Ws_2)*(Lr_2+Wr_2) - (Ls_2-Ws_2)*(Lr_2-Wr_2)*cos(2.0*alpha) ) ;
    double deg_2_m = 1852.0 * 60.0 ;
    double xs = abs(u->pos.latitude() - v->pos.latitude()) * deg_2_m ;
    double ys = abs(u->pos.longitude() - v->pos.longitude()) * deg_2_m ;
    double kappa = -0.25 * (xs*((Ls_2+Ws_2)+(Ls_2-Ws_2)*cos(2.0*alpha)+2.0*Lr_2)
                          + ys*((Ls_2+Ws_2)-(Ls_2-Ws_2)*cos(2.0*alpha)+2.0*Wr_2)
                          - xs*ys*(Ls_2-Ws_2)*sin(2.0*alpha)) / det_sr ;
    double _area = 0.5 * sqrt(det_r) * sqrt(det_s) * exp( kappa ) / sqrt(det_sr) ;
//    double range = v->pos.distance(u->pos) ;
//    double s2 = Lr_2 + Ls_2 ;
//    double _area = 0.5 * Lr_2 * Ls_2 * Ws_2 * Wr_2
//                    / sqrt( s2 * ( Wr_2 + Ws_2 ) )
//                    * exp( -0.5 * range * range / s2 ) ;
        // Compute the energy reflected off of this patch
    vector<double> _loss( (*u->frequencies).size() ) ;
    boundary->reflect_loss( u->pos, *(u->frequencies), u->grazing, &_loss ) ;
    _loss = pow( 10.0, -0.05 * _loss ) ;
    vector<double> TL_in = element_prod( u->intensity, _loss ) ;
    vector<double> two_way_TL = element_prod( TL_in, v->intensity ) ;
        // calculate the scattering loss from the interface
    vector<double> s_sr( (*v->frequencies).size() ) ;
    boundary->getScattering_Model()->scattering_strength( v->pos, (*v->frequencies),
            u->grazing, v->grazing, u->launch_az, v->launch_az, &s_sr ) ;
//    double _energy = _pulse * _loss(0) * u->intensity(0) * v->intensity(0) * s_sr(0) * _area ;
    double _energy = _pulse * two_way_TL(0) * s_sr(0) * _area ;

    #ifdef DEBUG_CONVOLUTION
        cout << "*****Eigenverb Convolution*****" << endl ;
        cout << "    Travel time:     " << u->time + v->time << endl ;
        cout << "        DE:          " << u->launch_de << endl ;
        cout << "     Path length:    " << u->distance << endl ;
        cout << "        xs:          " << xs << endl ;
        cout << "        ys:          " << ys << endl ;
        cout << "       Area:         " << _area << endl ;
        cout << "   grazing angle:    " << u->grazing << endl ;
        cout << "  reflection loss:   " << 10.0*log10(_loss) << endl ;
        cout << "      Loss in:       " << 10.0*log10(u->intensity) << endl ;
        cout << "       TL_in:        " << 10.0*log10(TL_in) << endl ;
        cout << "      Loss out:      " << 10.0*log10(v->intensity) << endl ;
        cout << "     Two-way TL:     " << 10.0*log10(two_way_TL) << endl ;
        cout << "scattering strength: " << 10.0*log10(s_sr) << endl ;
        cout << "      Energy:        " << 10.0*log10(_energy) << endl ;
    #endif

        // Only added value if contribution is significant
    if ( _energy > 1e-18 ) {
            // Calculate the time spread of the energy
//        double det_sr_inv = det_sr / (det_r*det_s) ;
//        double sigma_p_yy = 0.5 * ( (1.0/Ls_2+1.0/Ws_2)+(1.0/Ls_2-1.0/Ws_2)*cos(2.0*alpha)+2.0/Wr_2 ) / det_sr_inv ;
//        double Tarea = sqrt(sigma_p_yy)*cos(v->grazing) / v->c ;
        double sigma_p_yy = sqrt( 1.0 / ( 1.0/Ls_2 + 1.0/Lr_2 ) ) ;
        double Tarea = sigma_p_yy * sin(v->grazing) / v->c ;
        double Tsr = 0.5 * sqrt(_pulse*_pulse + Tarea*Tarea) ;
        double time = u->time + v->time + Tsr ;
        vector<double> time_exp = (_two_way_time-time) * ( 1.0 / Tsr ) ;
        time_exp = element_prod( time_exp, time_exp ) ;
        vector<double> _time_spread =  exp( -0.5 * time_exp ) * ( 1.0 / ( Tsr * sqrt(TWO_PI) ) ) ;
        #ifdef DEBUG_CONVOLUTION
            cout << "        Lp:         " << sigma_p_yy << endl ;
            cout << "      Tarea:        " << Tarea << endl ;
            cout << "       Tsr:         " << Tsr << endl ;
            cout << "       time:        " << time << endl ;
            cout << "     time diff:     " << _two_way_time-time << endl ;
            cout << "    time_spread:    " << _time_spread << endl ;
        #endif
        _time_spread *= _energy ;
        _reverberation_curve += _time_spread ;
    }
}
