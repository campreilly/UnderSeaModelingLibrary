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
//	TODO stub out while we update eigenverb_collection
//    #ifndef EIGENVERB_MODEL_DEBUG
//        cout << "**** Entering eigenverb_monostatic::compute_bottom_energy()"
//             << endl ;
//        cout << "Number of bottom eigenverbs: " << source.bottom().size() << endl ;
//    #endif
//    _current_boundary = _bottom_boundary ;
//    convolve_eigenverbs( source.bottom(), levels ) ;
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
	//	TODO stub out while we update eigenverb_collection
//    #ifndef EIGENVERB_MODEL_DEBUG
//        cout << "**** Entering eigenverb_monostatic::compute_surface_energy()"
//             << endl ;
//        cout << "Number of surface eigenverbs: " << source.surface().size() << endl ;
//    #endif
//    _current_boundary = _surface_boundary ;
//    convolve_eigenverbs( source.surface(), levels ) ;
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
    BOOST_FOREACH( eigenverb r, source )
            BOOST_FOREACH( eigenverb s, source )
                compute_contribution( &r, &s, levels ) ;
//    eigenverb_list rec ;
//    source.query( eigenverb_box(), &rec ) ;
//    BOOST_FOREACH( eigenverb r, rec )
//    {
//        eigenverb_list src ;
//        eigenverb_box ebox( r ) ;
//        source.query( ebox, &src ) ;
//        BOOST_FOREACH( eigenverb s, src )
//            compute_contribution( &r, &s, levels ) ;
//    }
}
