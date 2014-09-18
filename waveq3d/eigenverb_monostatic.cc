/**
 * @file eigenverb_monostatic.cc
 */

#include <usml/waveq3d/eigenverb_monostatic.h>

using namespace usml::waveq3d ;

/**  Constructor  **/
eigenverb_monostatic::eigenverb_monostatic( ocean_model& ocean,
    wave_queue_reverb& wave, double pulse, unsigned num_bins, double max_time )
{
        // Set all local member variables for use in calculations
    _pulse = pulse ;
    _max_index = num_bins-1 ;
    _max_time = max_time ;
        // extract pointers to various spreading/boundary model(s)
    _spreading_model = wave.getSpreading_Model() ;
    _bottom_boundary = &ocean.bottom() ;
    _surface_boundary = &ocean.surface() ;
    _volume_boundary = ocean.volume() ;
    if ( _volume_boundary ) {
        unsigned _n = ocean.volume()->getNumberOfLayers() ;
        _upper.resize( _n ) ;
        _lower.resize( _n ) ;
    }
        // Grab this wave's ID, used to determine volume layer interactions
    _source_origin = wave.getID() ;
        // Initialize all the memory of _reverberation_curve
    _reverberation_curve.resize( num_bins ) ;
    for(vector<double>::iterator i=_reverberation_curve.begin();
            i!=_reverberation_curve.end(); ++i) {
        (*i) = 1e-20 ;
    }
    _two_way_time.resize( num_bins ) ;
    double resolution = max_time / num_bins ;
    for(size_t i=0; i<num_bins; ++i) {
        _two_way_time[i] = i * resolution ;
    }
}

/**
 * Places an eigenverb into the class of "upper" bins to be used for the overall
 * reverberation calculation.
 */
void eigenverb_monostatic::notifyUpperCollision( unsigned de, unsigned az,
               double dt, double grazing, double speed,
               const wposition1& position, const wvector1& ndirection,
               const wave_queue& wave, unsigned ID )
{
    #ifdef EIGENVERB_COLLISION_DEBUG
        cout << "**** Entering eigenverb_monostatic::notifyUpperCollision" << endl ;
        cout << "de: " << de << " az: " << az << " time: " << time << endl ;
        cout << "grazing: " << grazing << " ID: " << ID << endl ;
    #endif
    eigenverb verb ;
    create_eigenverb( de, az, dt, grazing, speed, position, ndirection, wave, verb ) ;
        // Don't bother adding the ray it its too quiet
    if ( 1e-10 < verb.intensity(0) ) {
        switch (ID) {
            case SOURCE_ID:
                _surface.push_back( verb ) ;
                break ;
            default:
                unsigned layer = ID - _source_origin - 1 ;
                _upper.at(layer).push_back( verb ) ;
                break ;
        }
    }
}

/**
 * Places an eigenverb into the class of "lower" bins to be used for the overall
 * reverberation calculation.
 */
void eigenverb_monostatic::notifyLowerCollision( unsigned de, unsigned az,
               double dt, double grazing, double speed,
               const wposition1& position, const wvector1& ndirection,
               const wave_queue& wave, unsigned ID )
{
    #ifdef EIGENVERB_COLLISION_DEBUG
        cout << "**** Entering eigenverb_monostatic::notifyLowerCollision" << endl ;
        cout << "de: " << de << " az: " << az << " time: " << time << endl ;
        cout << "grazing: " << grazing << " ID: " << ID << endl ;
    #endif
    eigenverb verb ;
    create_eigenverb( de, az, dt, grazing, speed, position, ndirection, wave, verb ) ;
        // Don't bother adding the ray it its too quiet
    if ( 1e-10 < verb.intensity(0) ) {
        switch (ID) {
            case SOURCE_ID:
                _bottom.push_back( verb ) ;
                break ;
            default:
                unsigned layer = ID - _source_origin - 1 ;
                _lower.at(layer).push_back( verb ) ;
                break ;
        }
    }
}

/**
 * Computes the energy contributions to the reverberation
 * energy curve from the bottom interactions.
 */
void eigenverb_monostatic::compute_bottom_energy() {
    #ifndef EIGENVERB_MODEL_DEBUG
        cout << "**** Entering eigenverb_monostatic::compute_bottom_energy()"
             << endl ;
        cout << "Number of bottom eigenverbs: " << _bottom.size() << endl ;
    #endif
    convolve_eigenverbs( &_bottom, _bottom_boundary ) ;
}

/**
 * Computes the energy contributions to the reverberation
 * energy curve from the surface interactions.
 */
void eigenverb_monostatic::compute_surface_energy() {
    #ifndef EIGENVERB_MODEL_DEBUG
        cout << "**** Entering eigenverb_monostatic::compute_surface_energy()"
             << endl ;
        cout << "Number of surface eigenverbs: " << _surface.size() << endl ;
    #endif
//    convolve_eigenverbs( &_surface, _surface_boundary ) ;
}

/**
 * Compute all of the upper collision contributions due
 * to interactions with the volume layers
 */
void eigenverb_monostatic::compute_upper_volume_energy() {
    if ( _volume_boundary ) {
        #ifdef EIGENVERB_MODEL_DEBUG
            cout << "**** Entering eigenverb_monostatic::compute_upper_volume_energy()"
                 << endl ;
        #endif
        unsigned layer = 0 ;
        for(std::vector<std::vector<eigenverb> >::iterator k=_upper.begin();
                k!=_upper.end() && layer <= _n; ++k)
        {
            boundary_model* current_layer = _volume_boundary->getLayer(layer) ;
            convolve_eigenverbs( &(*k), current_layer) ;
            ++layer ;
        }
    }
}

/**
 * Compute all of the lower collision contributions due
 * to interactions with the volume layers
 */
void eigenverb_monostatic::compute_lower_volume_energy() {
    if ( _volume_boundary ) {
        #ifdef EIGENVERB_MODEL_DEBUG
            cout << "**** Entering eigenverb_monostatic::compute_lower_volume_energy()"
                 << endl ;
        #endif
        unsigned layer = 0 ;
        for(std::vector<std::vector<eigenverb> >::iterator k=_lower.begin();
                k!=_lower.end() && layer <= _n; ++k)
        {
            boundary_model* current_layer = _volume_boundary->getLayer(layer) ;
            convolve_eigenverbs( &(*k), current_layer) ;
            ++layer ;
        }
    }
}

/**
 * Convolution of the set of eigenverbs with itself from this boundary
 * and makes contributions to the reverberation levels curve when
 * a contribution is significant enough.
 */
void eigenverb_monostatic::convolve_eigenverbs( std::vector<eigenverb>* set,
                                                boundary_model* boundary )
{
    for(std::vector<eigenverb>::iterator i=set->begin();
            i!=set->end(); ++i)
    {
        for(std::vector<eigenverb>::iterator j=set->begin();
                j!=set->end(); ++j)
        {
//            if( i->az_index != j->az_index ) continue ;
//            if( (i->time + j->time) > _max_time ) continue ;
                // Don't make contributions anymore if the travel time
                // is greater then the max reverberation curve time
            compute_contribution( &(*i), &(*j), boundary ) ;
        }
    }
}

/**
 * Saves the eigenverb data to a text file
 */
void eigenverb_monostatic::save_eigenverbs(const char* filename) {
    std::ofstream of(filename) ;
    of << "                                     ************************\n"
       << "                                     * Eigenverb Monostatic *\n"
       << "                                     *       EIGENVERBS     *\n"
       << "                                     ************************\n" << endl ;
    of << "                             ===== Bottom Boundary Eigenverbs =====" << endl ;
    of << " Travel       Grazing       Launch      Launch      Path      Sigma      Sigma    Intensity    Frequency     Speed of   " << endl ;
    of << " time(s)    angle (deg)    DE (deg)    AZ (deg)   Length (m)    DE         AZ        (dB)         (Hz)      Sound (m/s) " << endl ;
    of << "---------  -------------  ----------  ---------- ----------- --------  ---------  ----------  -----------  -------------" << endl ;
    for(std::vector<eigenverb>::iterator b=_bottom.begin(); b!=_bottom.end(); ++b) {
        eigenverb e = (*b) ;
        char buff[256] ;
        sprintf( buff,"%s %1.4f  %12.4f  %11.3f  %9.1f  %10.2f %11.2f  %9.2f  %10.4f  %9.0f  %13.2f\n",
                 "", e.time, e.grazing*180.0/M_PI, e.launch_de, e.launch_az,
                 e.distance, e.sigma_de, e.sigma_az, 10.0*log10(e.intensity(0)),
                 (*e.frequencies)(0), e.c ) ;
        of << buff ;
    }
    of << endl ;
    of << "                             ===== Surface Boundary Eigenverbs =====" << endl ;
    of << " Travel       Grazing       Launch      Launch      Path      Sigma      Sigma    Intensity    Frequency     Speed of   " << endl ;
    of << " time(s)    angle (deg)    DE (deg)    AZ (deg)   Length (m)    DE         AZ        (dB)         (Hz)      Sound (m/s) " << endl ;
    of << "---------  -------------  ----------  ---------- ----------- --------  ---------  ----------  -----------  -------------" << endl ;
    for(std::vector<eigenverb>::iterator s=_surface.begin(); s!=_surface.end(); ++s) {
        eigenverb e = (*s) ;
        char buff[256] ;
        sprintf( buff,"%s %1.4f  %12.4f  %11.3f  %9.1f  %10.2f %11.2f  %9.2f  %10.4f  %9.0f  %13.2f\n",
                 "", e.time, e.grazing*180.0/M_PI, e.launch_de, e.launch_az,
                 e.distance, e.sigma_de, e.sigma_az, 10.0*log10(e.intensity(0)),
                 (*e.frequencies)(0), e.c ) ;
        of << buff ;
    }
    of << endl ;
    if( !_upper.empty() ) {
        int layer = 0 ;
        of << "                             ===== Volume Upper Boundary Eigenverbs =====" << endl ;
        of << " Travel       Grazing       Launch      Launch      Path      Sigma      Sigma    Intensity    Frequency     Speed of   " << endl ;
        of << " time(s)    angle (deg)    DE (deg)    AZ (deg)   Length (m)    DE         AZ        (dB)         (Hz)      Sound (m/s) " << endl ;
        of << "---------  -------------  ----------  ---------- ----------- --------  ---------  ----------  -----------  -------------" << endl ;
        for(std::vector<std::vector<eigenverb> >::iterator v=_upper.begin(); v!=_upper.end(); ++v, ++layer) {
            of << "Volume Layer " << layer << endl ;
            for(std::vector<eigenverb>::iterator u=v->begin(); u!=v->end(); ++u) {
                eigenverb e = (*u) ;
                char buff[256] ;
                sprintf( buff,"%s %1.4f  %12.4f  %11.3f  %9.1f  %10.4f %10.4f  %9.4f  %10.4f  %9.0f  %13.2f\n",
                         "", e.time, e.grazing*180.0/M_PI, e.launch_de, e.launch_az,
                         e.distance, e.sigma_de, e.sigma_az, 10.0*log10(e.intensity(0)),
                         (*e.frequencies)(0), e.c ) ;
                of << buff ;
            }
        }
    }
    of << endl ;
    if( !_lower.empty() ) {
        int layer = 0 ;
        of << "                             ===== Volume Lower Boundary Eigenverbs =====" << endl ;
        of << " Travel       Grazing       Launch      Launch      Path      Sigma      Sigma    Intensity    Frequency     Speed of   " << endl ;
        of << " time(s)    angle (deg)    DE (deg)    AZ (deg)   Length (m)    DE         AZ        (dB)         (Hz)      Sound (m/s) " << endl ;
        of << "---------  -------------  ----------  ---------- ----------- --------  ---------  ----------  -----------  -------------" << endl ;
        for(std::vector<std::vector<eigenverb> >::iterator v=_upper.begin(); v!=_upper.end(); ++v, ++layer) {
            of << "Volume Layer " << layer << endl ;
            for(std::vector<eigenverb>::iterator l=v->begin(); l!=v->end(); ++l) {
                eigenverb e = (*l) ;
                char buff[256] ;
                sprintf( buff,"%s %1.4f  %12.4f  %11.3f  %9.1f  %10.4f %10.4f  %9.4f  %10.4f  %9.0f  %13.2f\n",
                         "", e.time, e.grazing*180.0/M_PI, e.launch_de, e.launch_az,
                         e.distance, e.sigma_de, e.sigma_az, 10.0*log10(e.intensity(0)),
                         (*e.frequencies)(0), e.c ) ;
                of << buff ;
            }
        }
    }
}
