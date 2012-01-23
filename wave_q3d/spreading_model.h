/**
 * @file spreading_model.h
 * Spreading loss component of propagation loss.
 */
#ifndef USML_WAVEQ3D_SPREADING_MODEL_H
#define USML_WAVEQ3D_SPREADING_MODEL_H

#include <usml/wave_q3d/wave_queue.h>

namespace usml {
namespace wave_q3d {

using namespace usml::ocean ;

/**
 * @internal
 * A spreading loss model computes the spreading component of
 * propagation loss. These models compute their results as a
 * function of frequency to support broadband acoustics.
 */
class spreading_model {

    friend class wave_queue ;

  protected:

    /** Wavefront object associated with this model. */
    wave_queue& _wave ;

    /** Frequency dependent part of beam spreading. */
    vector<double> _spread ;

    /**
     * Initializes the spreading model.
     *
     * @param wave          Wavefront object associated with this model.
     */
    spreading_model( wave_queue& wave, unsigned num_freqs ) :
        _wave(wave), _spread(num_freqs)
    {}

    /**
     * Virtual destructor
     */
    virtual ~spreading_model() {}

    /**
     * Estimate intensity at a specific target location.
     *
     * @param  location     Target location.
     * @param  de           DE index of closest point of approach.
     * @param  az           AZ index of closest point of approach.
     * @param  offset       Offsets in time, DE, and AZ at collision.
     * @param  distance     Offsets in distance units.
     * @return              Intensity of ray at this point.
     */
    virtual const vector<double>& intensity( 
        const wposition1& location, unsigned de, unsigned az,
        const vector<double>& offset, const vector<double>& distance ) = 0 ;
} ;

}  // end of namespace wave_q3d
}  // end of namespace usml

#endif
