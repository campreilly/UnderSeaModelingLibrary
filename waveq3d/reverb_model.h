/**
 * @file reverb_model.h
 * Listens for interface collision callbacks from a wavefront.
 */
#ifndef USML_WAVEQ3D_REVERB_MODEL_H
#define USML_WAVEQ3D_REVERB_MODEL_H

#include <usml/ublas/ublas.h>
#include <usml/types/types.h>

namespace usml {
namespace waveq3d {

using namespace usml::ocean ;

using boost::numeric::ublas::vector;

/**
 * A reverberation model listens for interface collision callbacks from
 * a wavefront.
 *
 * @todo The reverb_model class is currently just a stub for future behaviors.
 */
class USML_DECLSPEC reverb_model {

  public:

    /**
     * React to the collision of a single ray with a reverberation surface.
     *
     * @param de            D/E angle index number.
     * @param az            AZ angle index number.
     * @param time          Time at which the collision occurs (sec)
     * @param position      Location at which the collision occurs (sec)
     * @param ndirection    Normalized direction at the point of collision.
     * @param speed         Speed of sound at the point of collision.
     * @param frequencies   Frequencies over which to compute reverb. (Hz)
     * @param amplitude     Propagation loss amplitude, at the point of
     *                      collision. as a function of frequency.
     * @param phase         Propagation loss phase at the point of
     *                      collision. as a function of frequency.
     */
    virtual void collision(
        unsigned de, unsigned az, double time,
        const wposition1& position, const wvector1& ndirection, double speed,
        const seq_vector& frequencies,
        const vector<double>& amplitude, const vector<double>& phase ) = 0 ;
};

/// @}
}  // end of namespace waveq3d
}  // end of namespace usml

#endif
