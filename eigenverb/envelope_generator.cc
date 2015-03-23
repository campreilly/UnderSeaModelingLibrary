/**
 * @file envelope_generator.cc
 */

#include <usml/eigenverb/envelope_generator.h>

//#define DEBUG_CONVOLUTION
using namespace usml::eigenverb ;

void envelope_generator::generate_envelopes(
    const eigenverb_collection& source,
    const eigenverb_collection& receiver,
    envelope_collection* levels )
{
    compute_bottom_energy( source, receiver, levels ) ;
    compute_surface_energy( source, receiver, levels ) ;
    if( source.volume() ) {
        compute_upper_volume_energy( source, receiver, levels ) ;
        compute_lower_volume_energy( source, receiver, levels ) ;
    }
}

/**
 * Computes the contribution value
 */
void envelope_generator::compute_contribution(
    const eigenverb* u, const eigenverb* v,
    envelope_collection* levels )
{
        // determine the relative angle and distance between the projected gaussians
    double alpha, chi, beta, dummy ;
    u->direction.direction( &dummy, &beta ) ;
    v->direction.direction( &dummy, &chi ) ;
    alpha = std::abs(std::fmod(chi,M_PI_2)) + std::abs(std::fmod(beta,M_PI_2)) ;
    double range = v->position.gc_range( u->position ) ;
    double xs = range * sin( alpha ) ;
    double ys = range * cos( alpha ) ;
    double cos_2alpha = cos( 2.0 * alpha ) ;

        // Compute the intersection of the gaussian profiles
    double Wr = v->sigma_az ;
    double Wr2 = Wr*Wr ;
    double Lr = v->sigma_de ;
    double Lr2 = Lr*Lr ;
    double Ws = u->sigma_az ;
    double Ws2 = Ws*Ws ;
    double Ls = u->sigma_de ;
    double Ls2 = Ls*Ls ;
    double s_minus = Ls2 - Ws2 ;
    double r_minus = Lr2 - Wr2 ;
    double s_plus = Ls2 + Ws2 ;
    double det_sr = 0.5 * ( 2.0*(Ls2*Ws2 + Lr2*Wr2) + s_plus*(Lr2+Wr2) - s_minus*r_minus*cos_2alpha ) ;
    double kappa = -0.25 * ( xs*xs*(s_plus + s_minus*cos_2alpha + 2.0*Lr2)
                             + ys*ys*(s_plus - s_minus*cos_2alpha + 2.0*Wr2)
                             - xs*ys*s_minus*sin(2.0*alpha) ) / det_sr ;
//    double det_sr = 0.5 * ( 2.0*(Ls2*Ws2 + Lr2*Wr2) + (Ls2+Ws2)*(Lr2+Wr2) - (Ls2-Ws2)*(Lr2-Wr2)*cos(2.0*alpha) ) ;
//    double kappa = -0.25 * (xs*xs*((Ls2+Ws2)+(Ls2-Ws2)*cos(2.0*alpha)+2.0*Lr2)
//                          + ys*ys*((Ls2+Ws2)-(Ls2-Ws2)*cos(2.0*alpha)+2.0*Wr2)
//                          - xs*ys*(Ls2-Ws2)*sin(2.0*alpha)) / det_sr ;
    double _area = 0.5 * Lr * Ls * Ws * Wr * exp( kappa ) / sqrt(det_sr) ;
        // Compute the energy reflected off of this patch
        // and the scattering loss from the interface
    vector<double> scatter( (*u->frequencies).size() ) ;
    _current_boundary->scattering( v->position, (*v->frequencies),
            u->grazing, v->grazing, u->launch_az, v->launch_az, &scatter ) ;
    double energy = _pulse * u->intensity(0) * v->intensity(0) * scatter(0) * _area ;

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
    if ( energy > 1e-18 ) {
            // Calculate the time spread of the energy
        double sigma_p_yy = ( Lr2 * ( Wr2*Ws2 + Ls2*(Wr2+2.0*Ws2) + Wr2*s_minus*cos_2alpha ) ) /
            ( Ls2*Wr2 + 2.0*Ls2*Ws2 + Wr2*Ws2 + Lr2*(Ls2+2.0*Wr2+Ws2) - r_minus*s_minus*cos_2alpha ) ;
//        double sigma_p_yy = ( Lr2 * ( Wr2*Ws2 + Ls2*(Wr2+2.0*Ws2) + Wr2*(Ls2-Ws2)*cos_2alpha ) ) /
//            ( Ls2*Wr2 + 2.0*Ls2*Ws2 + Wr2*Ws2 + Lr2*(Ls2+2.0*Wr2+Ws2) - (Lr2-Wr2)*(Ls2-Ws2)*cos_2alpha ) ;
        double Tarea = sqrt(sigma_p_yy) * sin(v->grazing) / v->sound_speed ;
        double Tsr = 0.5 * sqrt(_pulse*_pulse + Tarea*Tarea) ;
        double time = u->travel_time + v->travel_time + Tsr ;
        levels->add_gaussian( energy, time, Tsr, v->az_index ) ;
    }
}
