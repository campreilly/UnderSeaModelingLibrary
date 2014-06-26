/**
 * @file eigenverb.h
 * A single acoustic path between a source or receiver and a boundary.
 */
#ifndef USML_WAVEQ3D_EIGENVERB_H
#define USML_WAVEQ3D_EIGENVERB_H

#include <usml/types/types.h>

namespace usml {
namespace waveq3d {

using namespace usml::types ;

using boost::numeric::ublas::vector;

/// @ingroup waveq3d
/// @{

/**
 * A single acoustic path between a source or receiver and a boundary.
 */
struct eigenrverb {

    /**
     * One way travel time for this path (sec).
     */
    double time ;

    /**
     * One way transmission loss for this path (dB,positive).
     */
    vector< double > intensity ;

    /**
     * The grazing angle of this path at impact of the
     * boundary. (radians)
     */
    double grazing ;

    /**
     * Location of impact with the boundary.
     */
    wposition1 pos ;

    /**
     * Normalized derivative of the location at the impact.
     * This is used to compute the azimuthal angle between
     * two eigenverbs.
     */
    wvector1 ndir ;

    /**
     * Frequeinces of the wavefront (Hz)
     */
     const seq_vector* frequiences ;

    /**
     * The DE of the path from launch.
     */
    unsigned de ;

    /**
     * The AZ of the path from launch.
     */
    unsigned az ;

    /**
     * Sigma in the DE dimension
     */
    double sigma_de ;

    /**
     * Sigma in the AZ dimension
     */
    double sigma_az ;

    /**
     * The speed of sound at the point of impact
     * with the boundary.
     */
    double c ;

};

/// @}
}  // end of namespace waveq3d
}  // end of namespace usml

#endif
