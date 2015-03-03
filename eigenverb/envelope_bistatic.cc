/**
 * @file envelope_bistatic.cc
 */

#include <usml/eigenverb/envelope_bistatic.h>

using namespace usml::eigenverb ;

/**  Constructor  **/
envelope_bistatic::envelope_bistatic(
    ocean_model& ocean,
    double pulse,
    double max_time )
    : _bottom_boundary( &ocean.bottom() ),
      _surface_boundary( &ocean.surface() )
//      _volume_boundary( &ocean.volume() )
{
    _pulse = pulse ;
    _max_time = max_time ;
        // Set all local member variables for use in calculations
//    _pulse = pulse ;
//    _max_index = num_bins-1 ;
//    _max_time = max_time ;
//        // extract pointers to various spreading/boundary model(s)
//    _bottom_boundary = &ocean.bottom() ;
//    _surface_boundary = &ocean.surface() ;
//
//    size_t _n = ocean.num_volume() ;
//    if ( _n > 0 ) {
//        _receiver_upper.resize( _n ) ;
//        _receiver_lower.resize( _n ) ;
//        _source_upper.resize( _n ) ;
//        _source_lower.resize( _n ) ;
//    }
//
//        // Grab this wave's ID, used to determine volume layer interactions
//    _source_origin = wave_source.ID() ;
//    _receiver_origin = wave_receiver.ID() ;
//        // Initialize all the memory of _reverberation_curve
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
 * Destructor
 */
envelope_bistatic::~envelope_bistatic()
{

}

/**
 * Computes the energy contributions to the reverberation
 * energy curve from the bottom interactions.
 */
void envelope_bistatic::compute_bottom_energy(
    const eigenverb_collection& source,
    const eigenverb_collection& receiver,
    envelope_collection* levels )
{
    #ifdef EIGENVERB_MODEL_DEBUG
        cout << "**** Entering eigenverb_bistatic::compute_bottom_energy()"
             << endl ;
//        cout << "Number of source bottom verbs: " << _source_bottom.size() << endl ;
//        cout << "Number of receiver bottom verbs: " << _receiver_bottom.size() << endl ;
    #endif
    _current_boundary = _bottom_boundary ;
    convolve_eigenverbs( source.bottom(), receiver.bottom(), levels ) ;
}

/**
 * Computes the energy contributions to the reverberation
 * energy curve from the surface interactions.
 */
void envelope_bistatic::compute_surface_energy(
    const eigenverb_collection& source,
    const eigenverb_collection& receiver,
    envelope_collection* levels )
{
    #ifdef EIGENVERB_MODEL_DEBUG
        cout << "**** Entering eigenverb_bistatic::compute_surface_energy()"
             << endl ;
//        cout << "Number of source surface verbs: " << _source_surface.size() << endl ;
//        cout << "Number of receiver surface verbs: " << _receiver_surface.size() << endl ;
    #endif
    _current_boundary = _surface_boundary ;
    convolve_eigenverbs( source.surface(), receiver.surface(), levels ) ;
}

/**
 * Compute all of the upper collision contributions due
 * to interactions with the volume layers
 */
void envelope_bistatic::compute_upper_volume_energy(
    const eigenverb_collection& source,
    const eigenverb_collection& receiver,
    envelope_collection* levels )
{
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
void envelope_bistatic::compute_lower_volume_energy(
    const eigenverb_collection& source,
    const eigenverb_collection& receiver,
    envelope_collection* levels )
{
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
void envelope_bistatic::convolve_eigenverbs(
    const eigenverb_list& source,
    const eigenverb_list& receiver,
    envelope_collection* levels )
{
    BOOST_FOREACH( eigenverb s, source )
            BOOST_FOREACH( eigenverb r, receiver )
                compute_contribution( &s, &r, levels ) ;
//    for(std::vector<eigenverb>::iterator i=set1->begin();
//            i!=set1->end(); ++i)
//    {
//        for(std::vector<eigenverb>::iterator j=set2->begin();
//                j!=set2->end(); ++j)
//        {
//            compute_contribution( &(*i), &(*j) ) ;
//        }
//    }
}
