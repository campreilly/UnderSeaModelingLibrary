/**
 * @file eigenverb_bistatic.cc
 */

#include <usml/waveq3d/eigenverb_bistatic.h>

using namespace usml::waveq3d ;

/**  Constructor  **/
eigenverb_bistatic::eigenverb_bistatic( ocean_model& ocean,
    wave_queue_reverb& wave_source, wave_queue_reverb& wave_receiver,
    double pulse, size_t num_bins, double max_time )
{
        // Set all local member variables for use in calculations
    _pulse = pulse ;
    _max_index = num_bins-1 ;
    _max_time = max_time ;
        // extract pointers to various spreading/boundary model(s)
    _bottom_boundary = &ocean.bottom() ;
    _surface_boundary = &ocean.surface() ;

    size_t _n = ocean.num_volume() ;
    if ( _n > 0 ) {
        _receiver_upper.resize( _n ) ;
        _receiver_lower.resize( _n ) ;
        _source_upper.resize( _n ) ;
        _source_lower.resize( _n ) ;
    }

        // Grab this wave's ID, used to determine volume layer interactions
    _source_origin = wave_source.ID() ;
    _receiver_origin = wave_receiver.ID() ;
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
void eigenverb_bistatic::notifyUpperCollision( size_t de, size_t az,
               double dt, double grazing, double speed,
               const wposition1& position, const wvector1& ndirection,
               const wave_queue& wave, size_t ID )
{
    #ifdef EIGENVERB_COLLISION_DEBUG
        std::cout << "**** Entering eigenverb_bistatic::notifyUpperCollision()" << std::endl ;
        std::cout << "de: " << de << " az: " << az << " time: " << time << std::endl ;
        std::cout << "grazing: " << grazing << " ID: " << ID << std::endl ;
    #endif
    eigenverb verb ;
    create_eigenverb( de, az, dt, grazing, speed, position, ndirection, wave, verb ) ;
        // Don't bother adding the ray it its too quiet
    if ( 1e-20 < verb.intensity(0) ) {
        if( ID == _source_origin ) {
            _source_surface.push_back( verb ) ;
        } else
        if( ID == _receiver_origin ) {
            _receiver_surface.push_back( verb ) ;
        } else {
        	size_t layer ;
            if( ID > _receiver_origin ) {
                layer = ID - _receiver_origin - 1 ;
                _receiver_upper.at(layer).push_back( verb ) ;
            } else {
                layer = ID - _source_origin - 1 ;
                _source_upper.at(layer).push_back( verb ) ;
            }
        }
    }
}

/**
 * Places an eigenverb into the class of "lower" bins to be used for the overall
 * reverberation calculation.
 */
void eigenverb_bistatic::notifyLowerCollision( size_t de, size_t az,
               double dt, double grazing, double speed,
               const wposition1& position, const wvector1& ndirection,
               const wave_queue& wave, size_t ID )
{
    #ifdef EIGENVERB_COLLISION_DEBUG
        std::cout << "**** Entering eigenverb_bistatic::notifyLowerCollision()" << std::endl ;
        std::cout << "de: " << de << " az: " << az << " time: " << time << std::endl ;
        std::cout << "grazing: " << grazing << " ID: " << ID << std::endl ;
    #endif
    eigenverb verb ;
    create_eigenverb( de, az, dt, grazing, speed, position, ndirection, wave, verb ) ;
        // Don't bother adding the ray it its too quiet
    if ( 1e-20 < verb.intensity(0) ) {
        if( ID == _source_origin ) {
            _source_bottom.push_back( verb ) ;
        } else
        if( ID == _receiver_origin ) {
            _receiver_bottom.push_back( verb ) ;
        } else {
        	size_t layer ;
            if( ID > _receiver_origin ) {
                layer = ID - _receiver_origin - 1 ;
                _receiver_lower.at(layer).push_back( verb ) ;
            } else {
                layer = ID - _source_origin - 1 ;
                _source_lower.at(layer).push_back( verb ) ;
            }
        }
    }
}

/**
 * Computes the energy contributions to the reverberation
 * energy curve from the bottom interactions.
 */
void eigenverb_bistatic::compute_bottom_energy() {
    #ifdef EIGENVERB_MODEL_DEBUG
        cout << "**** Entering eigenverb_bistatic::compute_bottom_energy()"
             << endl ;
        cout << "Number of source bottom verbs: " << _source_bottom.size() << endl ;
        cout << "Number of receiver bottom verbs: " << _receiver_bottom.size() << endl ;
    #endif
    _current_boundary = _bottom_boundary ;
    convolve_eigenverbs( &_source_bottom, &_receiver_bottom ) ;
}

/**
 * Computes the energy contributions to the reverberation
 * energy curve from the surface interactions.
 */
void eigenverb_bistatic::compute_surface_energy() {
    #ifdef EIGENVERB_MODEL_DEBUG
        cout << "**** Entering eigenverb_bistatic::compute_surface_energy()"
             << endl ;
        cout << "Number of source surface verbs: " << _source_surface.size() << endl ;
        cout << "Number of receiver surface verbs: " << _receiver_surface.size() << endl ;
    #endif
    _current_boundary = _surface_boundary ;
    convolve_eigenverbs( &_source_surface, &_receiver_surface ) ;
}

/**
 * Compute all of the upper collision contributions due
 * to interactions with the volume layers
 */
void eigenverb_bistatic::compute_upper_volume_energy() {
//@todo Fix volume reverberation
//    if ( _volume_boundary ) {
//        #ifdef EIGENVERB_MODEL_DEBUG
//            cout << "**** Entering eigenverb_bistatic::compute_upper_volume_energy()"
//                 << endl ;
//        #endif
//        size_t layer = 0 ;
//        std::vector<std::vector<eigenverb> >::iterator i ;
//        std::vector<std::vector<eigenverb> >::iterator j ;
//        for(i=_source_upper.begin(), j=_receiver_upper.begin();
//            i!=_source_upper.end() && j!=_receiver_upper.end() && layer <= _n;
//            ++i, ++j)
//        {
//            _current_boundary = _volume_boundary->getLayer(layer) ;
//            convolve_eigenverbs( &(*i), &(*j) ) ;
//            ++layer ;
//        }
//    }
}

/**
 * Compute all of the lower collision contributions due
 * to interactions with the volume layers
 */
void eigenverb_bistatic::compute_lower_volume_energy() {
//@todo Fix volume reverberation
//    if ( _volume_boundary ) {
//        #ifdef EIGENVERB_MODEL_DEBUG
//            cout << "**** Entering eigenverb_bistatic::compute_lower_volume_energy()"
//                 << endl ;
//        #endif
//        size_t layer = 0 ;
//        std::vector<std::vector<eigenverb> >::iterator i ;
//        std::vector<std::vector<eigenverb> >::iterator j ;
//        for(i=_source_lower.begin(), j=_receiver_lower.begin();
//            i!=_source_lower.end() && j!=_receiver_lower.end() && layer <= _n;
//            ++i, ++j)
//        {
//            _current_boundary = _volume_boundary->getLayer(layer) ;
//            convolve_eigenverbs( &(*i), &(*j) ) ;
//            ++layer ;
//        }
//    }
}

/**
 * Convolution of the set of eigenverbs with itself from this boundary
 * and makes contributions to the reverberation levels curve when
 * a contribution is significant enough.
 */
void eigenverb_bistatic::convolve_eigenverbs( std::vector<eigenverb>* set1,
            std::vector<eigenverb>* set2 )
{
    for(std::vector<eigenverb>::iterator i=set1->begin();
            i!=set1->end(); ++i)
    {
        for(std::vector<eigenverb>::iterator j=set2->begin();
                j!=set2->end(); ++j)
        {
            compute_contribution( &(*i), &(*j) ) ;
        }
    }
}
