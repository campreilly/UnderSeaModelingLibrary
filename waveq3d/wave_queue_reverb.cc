/**
 * @file wave_queue_reverb.cc
 * Wavefront propagation as a function of time.
 */
#include <usml/waveq3d/wave_queue_reverb.h>
#include <usml/waveq3d/ode_integ.h>
#include <usml/waveq3d/reflection_model.h>
#include <usml/waveq3d/spreading_ray.h>
#include <usml/waveq3d/spreading_hybrid_gaussian.h>

#include <boost/numeric/ublas/vector_proxy.hpp>
#include <boost/numeric/ublas/triangular.hpp>
#include <boost/numeric/ublas/lu.hpp>

#include <iomanip>

using namespace usml::waveq3d ;

/**
 * Initialize a propagation scenario.
 */
wave_queue_reverb::wave_queue_reverb(
    ocean_model& ocean,
    const seq_vector& freq,
    const wposition1& pos,
    const seq_vector& de,
    const seq_vector& az,
    double time_step,
    double pulse,
    unsigned num_bins,
    double max_time,
    reverberation_model* reverb,
    unsigned origin,
    const wposition* targets,
    spreading_type type
) :
    wave_queue(ocean, freq, pos, de, az, time_step, targets, type),
    _origin( origin )
{
    // Define reverberation model
    if ( reverb ) { _reverberation_model = reverb ; }
    else {
        _reverberation_model = new
        eigenverb_monostatic( ocean, *this, pulse, num_bins, max_time ) ;
    }
    delete _reflection_model ;
    _reflection_model = new reflection_model( *this, _reverberation_model ) ;
}

/**
 * Detect and process boundary reflections and caustics.
 */
void wave_queue_reverb::detect_reflections() {

    // process all surface and bottom reflections, and vertices
    // note that multiple rays can reflect in the same time step

    for (unsigned de = 0; de < num_de(); ++de) {
        for (unsigned az = 0; az < num_az(); ++az) {
            if ( !detect_reflections_surface(de,az) ) {
                if( !detect_reflections_bottom(de,az) ) {
                    detect_vertices(de,az) ;
                }
            }
        }
    }

    detect_volume_reflections() ;

    // search for other changes in wavefront
    _next->find_edges() ;
    detect_caustics() ;
}

/**
 * Detect volume boundary reflections for reverberation contributions
 */
void wave_queue_reverb::detect_volume_reflections() {

}
