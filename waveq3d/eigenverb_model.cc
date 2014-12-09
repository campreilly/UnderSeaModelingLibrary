/**
 * @file eigenverb_model.h
 */

#include <usml/waveq3d/eigenverb_model.h>

//#define DEBUG_CONVOLUTION

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
    verb.travel_time = wave.time() + dt ;
    verb.grazing = grazing ;
    verb.sound_speed = speed ;
    verb.position = position ;
    verb.direction = ndirection ;
    verb.frequencies = wave.frequencies() ;
    verb.surface = wave.curr()->getSurface(de,az) ;
    verb.bottom = wave.curr()->getBottom(de,az) ;

        // Calculate the one way TL and the width of the gaussian
        // at the time of impact with the boundary.
    double true_distance = verb.distance ;
    double spreading_loss = 1.0 / (true_distance * true_distance) ;
    vector<double> amp( wave.frequencies()->size(), spreading_loss ) ;
    vector<double> boundary_loss = pow( 10.0, -0.1 * wave.curr()->getAttenuation(de,az) ) ;
    verb.intensity = element_prod( amp, boundary_loss ) ;

    double delta_de ;
    if( de == 0 ) {
        delta_de = M_PI * ( wave.source_de(de+1) - wave.source_de(de) ) / 180.0  ;
    } else {
        delta_de = M_PI * ( wave.source_de(de+1) - wave.source_de(de-1) ) / 360.0  ;
    }
    verb.sigma_de = true_distance * delta_de / sin(grazing) ;
    double delta_az = M_PI * ( wave.source_az(az+1) - wave.source_az(az) ) / 180.0 ;
    verb.sigma_az = delta_az * cos(grazing) * true_distance ;   // horizontal distance * azimuthal spacing
    if( abs(grazing) > (M_PI_2 - 1e-10) ) verb.sigma_az = TWO_PI * true_distance ;

    #ifdef EIGENVERB_COLLISION_DEBUG
        cout << "\t---Added eigenverb to collection---" << endl ;
        cout << "\tverb de: " << verb.launch_de << " az: " << verb.launch_az
             << " time: " << verb.travel_time << endl ;
        cout << "\tgrazing: " << verb.grazing*180.0/M_PI << " speed: " << verb.sound_speed << endl ;
        cout << "\tintensity: " << verb.intensity << " sigma_de: " << verb.sigma_de
             << " sigma_az: " << verb.sigma_az << endl ;
    #endif
}

/**
 * Computes the contribution value
 */
void eigenverb_model::compute_contribution( const eigenverb* u, const eigenverb* v )
{
        // determine the relative angle and distance between the projected gaussians
    double alpha, chi, beta, dummy ;
    u->direction.direction( &dummy, &beta ) ;
    v->direction.direction( &dummy, &chi ) ;
    alpha = std::abs(std::fmod(chi,M_PI_2)) + std::abs(std::fmod(beta,M_PI_2)) ;
    double range = v->position.gc_range( u->position ) ;
    double xs = range * sin( alpha ) ;
    double ys = range * cos( alpha ) ;

        // Compute the intersection of the gaussian profiles
    double Wr = v->sigma_az ;
    double Wr2 = Wr*Wr ;
    double Lr = v->sigma_de ;
    double Lr2 = Lr*Lr ;
    double Ws = u->sigma_az ;
    double Ws2 = Ws*Ws ;
    double Ls = u->sigma_de ;
    double Ls2 = Ls*Ls ;
    double det_sr = 0.5 * ( 2.0*(Ls2*Ws2 + Lr2*Wr2) + (Ls2+Ws2)*(Lr2+Wr2) - (Ls2-Ws2)*(Lr2-Wr2)*cos(2.0*alpha) ) ;
    double kappa = -0.25 * (xs*xs*((Ls2+Ws2)+(Ls2-Ws2)*cos(2.0*alpha)+2.0*Lr2)
                          + ys*ys*((Ls2+Ws2)-(Ls2-Ws2)*cos(2.0*alpha)+2.0*Wr2)
                          - xs*ys*(Ls2-Ws2)*sin(2.0*alpha)) / det_sr ;
    double _area = 0.5 * Lr * Ls * Ws * Wr * exp( kappa ) / sqrt(det_sr) ;
        // Compute the energy reflected off of this patch
        // and the scattering loss from the interface
    vector<double> scatter( (*u->frequencies).size() ) ;
    _current_boundary->scattering( v->position, (*v->frequencies),
            u->grazing, v->grazing, u->launch_az, v->launch_az, &scatter ) ;
    double _energy = _pulse * u->intensity(0) * v->intensity(0) * scatter(0) * _area ;

    #ifdef DEBUG_CONVOLUTION
        cout << "*****Eigenverb Convolution*****" << endl ;
        cout << "    Travel time:     " << u->travel_time + v->travel_time << endl ;
        cout << "        DE:          " << u->launch_de << endl ;
        cout << "     Path length:    " << u->distance << endl ;
        cout << "       range:        " << range << endl ;
        cout << "        xs:          " << xs << endl ;
        cout << "        ys:          " << ys << endl ;
        cout << "       Area:         " << _area << endl ;
        cout << "   grazing angle:    " << u->grazing*180.0/M_PI << endl ;
        cout << "      Loss in:       " << 10.0*log10(u->intensity) << endl ;
        cout << "      Loss out:      " << 10.0*log10(v->intensity) << endl ;
        cout << "     Two-way TL:     " << 10.0*log10(element_prod(u->intensity, v->intensity)) << endl ;
        cout << "scattering strength: " << 10.0*log10(scatter) << endl ;
        cout << "      Energy:        " << 10.0*log10(_energy) << endl ;
    #endif

        // Only added value if contribution is significant
    if ( _energy > 1e-18 ) {
            // Calculate the time spread of the energy
        double sigma_p_yy = ( Lr2 * ( Wr2*Ws2 + Ls2*(Wr2+2.0*Ws2) + Wr2*(Ls2-Ws2)*cos(2.0*alpha) ) ) /
            ( Ls2*Wr2 + 2.0*Ls2*Ws2 + Wr2*Ws2 + Lr2*(Ls2+2.0*Wr2+Ws2) - (Lr2-Wr2)*(Ls2-Ws2)*cos(2.0*alpha) ) ;
        double Tarea = sqrt(sigma_p_yy) * sin(v->grazing) / v->sound_speed ;
        double Tsr = 0.5 * sqrt(_pulse*_pulse + Tarea*Tarea) ;
        double time = u->travel_time + v->travel_time + Tsr ;
        vector<double> time_exp = (_two_way_time-time) * ( 1.0 / Tsr ) ;
        time_exp = element_prod( time_exp, time_exp ) ;
        vector<double> _time_spread = ( _energy / ( Tsr * sqrt(TWO_PI) ) ) * exp( -0.5 * time_exp ) ;
        #ifdef DEBUG_CONVOLUTION
            cout << "        Lp:         " << sigma_p_yy << endl ;
            cout << "      Tarea:        " << Tarea << endl ;
            cout << "       Tsr:         " << Tsr << endl ;
            cout << "       time:        " << time << endl ;
            cout << "     time diff:     " << _two_way_time-time << endl ;
            cout << "    time_spread:    " << _time_spread << endl ;
        #endif
        _reverberation_curve += _time_spread ;
    }
}
