/**
 * @file envelope_monostatic.cc
 */

#include <usml/eigenverb/envelope_monostatic.h>

using namespace usml::eigenverb ;

/**  Constructor  **/
envelope_monostatic::envelope_monostatic(
    ocean_model& ocean,
    double pulse,
    double max_time )
    : _bottom_boundary( &ocean.bottom() ),
      _surface_boundary( &ocean.surface() )
//      _volume_boundary( &ocean.volume() )
{
    _pulse = pulse ;
    _max_time = max_time ;
//        // Set all local member variables for use in calculations
//    _pulse = pulse ;
//    _max_index = num_bins-1 ;
//    _max_time = max_time ;
//        // extract pointers to various spreading/boundary model(s)
//    _bottom_boundary = &ocean.bottom() ;
//    _surface_boundary = &ocean.surface() ;
////    _volume_boundary = &ocean.volume() ;
//
//    // Initialize all the memory of _reverberation_curve
//    _reverberation_curve.resize( num_bins ) ;
//    for(vector<double>::iterator i=_reverberation_curve.begin();
//            i!=_reverberation_curve.end(); ++i) {
//        (*i) = 1e-20 ;
//    }
//    _two_way_time.resize( num_bins ) ;
//    double resolution = max_time / num_bins ;
//    for(size_t i=0; i<num_bins; ++i) {
//        _two_way_time[i] = i * resolution ;
//    }
}

/**
 * Computes the energy contributions to the reverberation
 * energy curve from the bottom interactions.
 */
void envelope_monostatic::compute_bottom_energy(
        const eigenverb_collection& source,
        const eigenverb_collection& receiver,
        envelope_collection* levels )
{
    #ifdef EIGENVERB_MODEL_DEBUG
        cout << "**** Entering eigenverb_monostatic::compute_bottom_energy()"
             << endl ;
        cout << "Number of bottom eigenverbs: " << source.bottom.size() << endl ;
    #endif
    _current_boundary = _bottom_boundary ;
    convolve_eigenverbs( source.bottom(), levels ) ;
}

/**
 * Computes the energy contributions to the reverberation
 * energy curve from the surface interactions.
 */
void envelope_monostatic::compute_surface_energy(
    const eigenverb_collection& source,
    const eigenverb_collection& receiver,
    envelope_collection* levels )
{
    #ifdef EIGENVERB_MODEL_DEBUG
        cout << "**** Entering eigenverb_monostatic::compute_surface_energy()"
             << endl ;
        cout << "Number of surface eigenverbs: " << source.surface.size() << endl ;
    #endif
    _current_boundary = _surface_boundary ;
    convolve_eigenverbs( source.surface(), levels ) ;
}

/**
 * Compute all of the upper collision contributions due
 * to interactions with the volume layers
 */
void envelope_monostatic::compute_upper_volume_energy(
    const eigenverb_collection& source,
    const eigenverb_collection& receiver,
    envelope_collection* levels )
{
//@todo Fix volume reverberation
//    if ( _volume_boundary ) {
//        #ifdef EIGENVERB_MODEL_DEBUG
//            cout << "**** Entering eigenverb_monostatic::compute_upper_volume_energy()"
//                 << endl ;
//        #endif
//        size_t layer = 0 ;
//        for(std::vector<std::vector<eigenverb> >::iterator k=_upper.begin();
//                k!=_upper.end() && layer <= _n; ++k)
//        {
//            _current_boundary = _volume_boundary->getLayer(layer) ;
//            convolve_eigenverbs( &(*k) ) ;
//            ++layer ;
//        }
//    }
}

/**
 * Compute all of the lower collision contributions due
 * to interactions with the volume layers
 */
void envelope_monostatic::compute_lower_volume_energy(
    const eigenverb_collection& source,
    const eigenverb_collection& receiver,
    envelope_collection* levels )
{
//@todo Fix volume reverberation
//    if ( _volume_boundary ) {
//        #ifdef EIGENVERB_MODEL_DEBUG
//            cout << "**** Entering eigenverb_monostatic::compute_lower_volume_energy()"
//                 << endl ;
//        #endif
//        size_t layer = 0 ;
//        for(std::vector<std::vector<eigenverb> >::iterator k=_lower.begin();
//                k!=_lower.end() && layer <= _n; ++k)
//        {
//            _current_boundary = _volume_boundary->getLayer(layer) ;
//            convolve_eigenverbs( &(*k) ) ;
//            ++layer ;
//        }
//    }
}

/**
 * Convolution of the set of eigenverbs with itself from this boundary
 * and makes contributions to the reverberation levels curve when
 * a contribution is significant enough.
 */
void envelope_monostatic::convolve_eigenverbs(
    const eigenverb_list& source,
    envelope_collection* levels )
{
//    double a, b, r, x, y, scale_x, scale_y ;
//    double lat1, lat2, alpha_s, ir, is ;
//    double k = 1852.0 * 60.0 ;
//    double threshold = 1e-18 ;
    BOOST_FOREACH( eigenverb s, source )
        BOOST_FOREACH( eigenverb r, source )
            compute_contribution( &r, &s, levels ) ;
//    for(std::vector<eigenverb>::iterator i=set->begin();
//            i!=set->end(); ++i)
//    {
            //rayfan
//        scale_x = (i->sigma_de<400.0) ? 1.8 : 0.1 ;
//        scale_y = (i->sigma_az<400.0) ? 1.8 : 0.1 ;
            //linear
//        scale_x = (i->sigma_de<3000.0) ? 0.65 : 0.2 ;
//        scale_y = (i->sigma_az<3000.0) ? 0.65 : 0.2 ;
//        scale_x = 1.0 ;
//        scale_y = 1.0 ;
//        scale_x *= i->sigma_de ;
//        scale_y *= i->sigma_az ;
//        ir = i->intensity(0) ;
//        if( i->de_index == 91 ) continue ;
//        for(std::vector<eigenverb>::iterator j=set->begin();
//                j!=set->end(); ++j)
//        {
//            is = j->intensity(0) ;
//            if( is*ir < threshold ) continue ;
//            lat1 = i->position.latitude() ;
//            lat2 = j->position.latitude() ;
//            a = ( lat1 - lat2 ) * k ;
//            b = ( j->position.longitude() - i->position.longitude() ) * k * cos(0.5*(lat2+lat1)) ;
//            alpha_s = std::abs(std::fmod(std::atan2( i->direction.phi(), -i->direction.theta() ), M_PI_2))
//                    + std::abs(std::fmod(std::atan2( j->direction.phi(), -j->direction.theta() ), M_PI_2)) ;
//            r = std::sqrt(a*a + b*b) ;
//            x = r * sin(alpha_s) ;
//            if( x > scale_x ) continue ;            // this gaussian is more then 5 sigma away in the x direction
//            y = r * cos(alpha_s) ;
//            if( y > scale_y ) continue ;            // this gaussian is more then 5 sigma away in the y direction
//            if( j->de_index == 91 ) continue ;
//            compute_contribution( &(*i), &(*j) ) ;
//        }
//    }
}

/**
 * Saves the eigenverb data to a text file
 */
void envelope_monostatic::save_eigenverbs(const char* filename) {
//    std::ofstream of(filename) ;
//    of << "                                     ************************\n"
//       << "                                     * Eigenverb Monostatic *\n"
//       << "                                     *       EIGENVERBS     *\n"
//       << "                                     ************************\n" << endl ;
//    of << "                             ===== Bottom Boundary Eigenverbs =====" << endl ;
//    of << " Travel    Collisions    Grazing       Launch      Launch      Path      Sigma      Sigma    Intensity    Frequency     Speed of   " << endl ;
//    of << " time(s)     (b,s)     angle (deg)    DE (deg)    AZ (deg)   Length (m)    DE         AZ        (dB)         (Hz)      Sound (m/s) " << endl ;
//    of << "---------  ---------- -------------  ----------  ---------- ----------- --------  ---------  ----------  -----------  -------------" << endl ;
//    for(std::vector<eigenverb>::iterator b=_bottom.begin(); b!=_bottom.end(); ++b) {
//        eigenverb e = (*b) ;
//        char buff[256] ;
//        sprintf( buff,"%s %1.4f %5s %i,%i %13.4f  %12.3f  %9.1f  %10.2f %10.2f  %9.2f  %10.4f  %9.0f  %13.2f\n",
//                 "", e.travel_time, "", e.bottom, e.surface, e.grazing*180.0/M_PI, e.launch_de, e.launch_az,
//                 e.distance, e.sigma_de, e.sigma_az, 10.0*log10(e.intensity(0)),
//                 (*e.frequencies)(0), e.sound_speed ) ;
//        of << buff ;
//    }
//    of << endl ;
//    of << "                             ===== Surface Boundary Eigenverbs =====" << endl ;
//    of << " Travel    Collisions    Grazing       Launch      Launch      Path      Sigma      Sigma    Intensity    Frequency     Speed of   " << endl ;
//    of << " time(s)     (b,s)     angle (deg)    DE (deg)    AZ (deg)   Length (m)    DE         AZ        (dB)         (Hz)      Sound (m/s) " << endl ;
//    of << "---------  ---------- -------------  ----------  ---------- ----------- --------  ---------  ----------  -----------  -------------" << endl ;
//    for(std::vector<eigenverb>::iterator s=_surface.begin(); s!=_surface.end(); ++s) {
//        eigenverb e = (*s) ;
//        char buff[256] ;
//        sprintf( buff,"%s %1.4f %4s %i,%i %12.4f  %11.3f  %9.1f  %10.2f %11.2f  %9.2f  %10.4f  %9.0f  %13.2f\n",
//                 "", e.travel_time, "", e.bottom, e.surface, e.grazing*180.0/M_PI, e.launch_de, e.launch_az,
//                 e.distance, e.sigma_de, e.sigma_az, 10.0*log10(e.intensity(0)),
//                 (*e.frequencies)(0), e.sound_speed ) ;
//        of << buff ;
//    }
//    of << endl ;
//    if( !_upper.empty() ) {
//        int layer = 0 ;
//        of << "                             ===== Volume Upper Boundary Eigenverbs =====" << endl ;
//        of << " Travel    Collisions    Grazing       Launch      Launch      Path      Sigma      Sigma    Intensity    Frequency     Speed of   " << endl ;
//        of << " time(s)     (b,s)     angle (deg)    DE (deg)    AZ (deg)   Length (m)    DE         AZ        (dB)         (Hz)      Sound (m/s) " << endl ;
//        of << "---------  ---------- -------------  ----------  ---------- ----------- --------  ---------  ----------  -----------  -------------" << endl ;
//        for(std::vector<std::vector<eigenverb> >::iterator v=_upper.begin(); v!=_upper.end(); ++v, ++layer) {
//            of << "Volume Layer " << layer << endl ;
//            for(std::vector<eigenverb>::iterator u=v->begin(); u!=v->end(); ++u) {
//                eigenverb e = (*u) ;
//                char buff[256] ;
//                sprintf( buff,"%s %1.4f %4s %i,%i %12.4f  %11.3f  %9.1f  %10.2f %11.2f  %9.2f  %10.4f  %9.0f  %13.2f\n",
//                         "", e.travel_time, "", e.bottom, e.surface, e.grazing*180.0/M_PI, e.launch_de, e.launch_az,
//                         e.distance, e.sigma_de, e.sigma_az, 10.0*log10(e.intensity(0)),
//                         (*e.frequencies)(0), e.sound_speed ) ;
//                of << buff ;
//            }
//        }
//    }
//    of << endl ;
//    if( !_lower.empty() ) {
//        int layer = 0 ;
//        of << "                             ===== Volume Lower Boundary Eigenverbs =====" << endl ;
//        of << " Travel    Collisions    Grazing       Launch      Launch      Path      Sigma      Sigma    Intensity    Frequency     Speed of   " << endl ;
//        of << " time(s)     (b,s)     angle (deg)    DE (deg)    AZ (deg)   Length (m)    DE         AZ        (dB)         (Hz)      Sound (m/s) " << endl ;
//        of << "---------  ---------- -------------  ----------  ---------- ----------- --------  ---------  ----------  -----------  -------------" << endl ;
//        for(std::vector<std::vector<eigenverb> >::iterator v=_upper.begin(); v!=_upper.end(); ++v, ++layer) {
//            of << "Volume Layer " << layer << endl ;
//            for(std::vector<eigenverb>::iterator l=v->begin(); l!=v->end(); ++l) {
//                eigenverb e = (*l) ;
//                char buff[256] ;
//                sprintf( buff,"%s %1.4f %4s %i,%i %12.4f  %11.3f  %9.1f  %10.2f %11.2f  %9.2f  %10.4f  %9.0f  %13.2f\n",
//                         "", e.travel_time, "", e.bottom, e.surface, e.grazing*180.0/M_PI, e.launch_de, e.launch_az,
//                         e.distance, e.sigma_de, e.sigma_az, 10.0*log10(e.intensity(0)),
//                         (*e.frequencies)(0), e.sound_speed ) ;
//                of << buff ;
//            }
//        }
//    }
}
