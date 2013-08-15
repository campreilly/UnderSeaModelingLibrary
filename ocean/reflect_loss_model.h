/** 
 * @file reflect_loss_model.h
 * Generic interface for reflection loss models.
 */
#ifndef USML_OCEAN_REFLECT_LOSS_MODEL_H
#define USML_OCEAN_REFLECT_LOSS_MODEL_H

#include <usml/ublas/ublas.h>
#include <usml/types/types.h>

namespace usml {
namespace ocean {

using namespace usml::ublas ;
using namespace usml::types ;

/// @ingroup boundaries
/// @{

/**
 * A reflection loss model computes the changes in amplitude and phase
 * that result from the reflection of a ray from a boundary.
 * Reflection is calculated on a ray-by-ray basis instead of across the
 * wavefront because only selected rays interact with the boundary at any
 * given time step. These models compute their results as a function 
 * of frequency to support broadband acoustics.  
 */
class USML_DECLSPEC reflect_loss_model {

public:

    /**
     * Computes the broadband reflection loss and phase change.
     *
     * @param location      Location at which to compute attenuation.
     * @param frequencies   Frequencies over which to compute loss. (Hz)
     * @param angle         Grazing angle relative to the interface (radians).
     * @param amplitude     Change in ray strength in dB (output).
     * @param phase         Change in ray phase in radians (output).
     *                      Phase change not computed if this is NULL.
     */
    virtual void reflect_loss( 
        const wposition1& location, 
        const seq_vector& frequencies, double angle,
        boost::numeric::ublas::vector<double>* amplitude, boost::numeric::ublas::vector<double>* phase=NULL ) = 0 ;

	/**
	 * Virtual destructor
	 */
	virtual ~reflect_loss_model() {}
} ;

/// @}
}  // end of namespace ocean
}  // end of namespace usml

#endif
