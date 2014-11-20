/** 
 * @file attenuation_model.h
 * Generic interface for attenuation loss models.
 */
#pragma once

#include <usml/ublas/ublas.h>
#include <usml/types/types.h>

namespace usml {
namespace ocean {

using namespace usml::ublas ;
using namespace usml::types ;

using boost::numeric::ublas::vector;

/// @ingroup profiles
/// @{

/**
 * An attenuation loss model computes the absorption of sound
 * by the minerals in sea water.  The attenuation is calculated as a 
 * dB factor that gets applied to the acoustic ray at each range step. 
 * These models compute their results as a function of frequency 
 * to support broadband acoustics.  
 */
class USML_DECLSPEC attenuation_model {

public:

    /**
     * Computes the broadband absorption loss of sea water.
     *
     * @param location      Location at which to compute attenuation.
     * @param frequencies   Frequencies over which to compute loss. (Hz)
     * @param distance      Distance travelled through the water (meters).
     * @param attenuation   Absorption loss of sea water in dB (output).
     */
    virtual void attenuation( 
        const wposition& location, 
        const seq_vector& frequencies,
        const matrix<double>& distance,
        matrix< vector<double> >* attenuation ) = 0 ;

	/**
	 * Virtual destructor
	 */
	virtual ~attenuation_model() {}
} ;

/// @}
}  // end of namespace ocean
}  // end of namespace usml
