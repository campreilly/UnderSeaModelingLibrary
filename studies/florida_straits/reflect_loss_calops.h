/** 
 * @file reflect_loss_calops.h
 * Models plane wave reflection from a flat fluid-solid interface.
 * Includes LaTEX documentation of formula for processing by Doxygen.
 */

#ifndef REFLECT_LOSS_CALOPS_H
#define REFLECT_LOSS_CALOPS_H

#include <usml/ocean/reflect_loss_model.h>

/**
 * Models the bottom reflection properties of the CALOPS experiment
 * in the Straits of Florida. The Calibration Operations (CALOPS)
 * experiment was conducted in the area from 26-26.8N 80-79.85W
 * in the late summer (September 7–15) of 2007 and in the
 * winter (February 19–25) of 2008. The geophysical properties
 * for this model were taken from Ballard’s analysis of
 * measurements around this site.
 * 
 * Below the 236 m isobaths, the bottom is bare limestone because
 * loose sediments have been scoured off by the Florida Current.
 * At shallower depths, carbonate sand sediments layers cover the bottom.
 * Although Ballard’s analysis includes some area of sediment
 * pooling in the limestone area, this refinement will be saved
 * for a follow-on effort.  The large shear speeds in the limestone
 * result in higher bottom loss, compared to the sand, at
 * grazing angles below 20 degrees.  Although the carbonate sands
 * may have shear speeds as high as 200 m/s, this has little effect
 * on the low angle bottom loss.
 *
 * @xref K. D. Heaney and J. J. Murray, "Measurements of three-dimensional
 * propagation in a continental shelf environment," J. Acoust. Soc. Am.,
 * vol. 3, no. 125, pp. 1394-1402, March 2009.
 * @xref M.S. Ballard, Modeling three-dimensional propagation in a
 * continental shelf environment, J. Acoust. Soc. Am. 131 (3), March 2012.
 */
class USML_DECLSPEC reflect_loss_calops : public reflect_loss_model {

  private:

    /** Reflection loss model above the the 236 m isobath. */
    reflect_loss_model* carbonate_sand ;

    /** Reflection loss model below the the 236 m isobath. */
    reflect_loss_model* limestone ;

  public:

    /**
     * Initialize model with Rayleigh reflection loss models for
     * both carbonate sand and limestone.
     */
    reflect_loss_calops() {
        carbonate_sand = new reflect_loss_rayleigh( 1.70, 1675.0/1500.0, 0.8, 0.0, 0.0 ) ;
        limestone = new reflect_loss_rayleigh( 2.40, 3000.0/1500.0, 0.1, 1430.0/1500.0, 0.2 ) ;
    }


    /**
     * Computes the broadband reflection loss and phase change.
     * Uses the carbonate sands model for depths shallower than 236 m.
     * Uses the limestone model for deeper locations.
     *
     * @param location      Location at which to compute attenuation.
     * @param frequencies   Frequencies over which to compute loss. (Hz)
     * @param angle         Reflection angle relative to the normal (radians).
     * @param amplitude     Change in ray strength in dB (output).
     * @param phase         Change in ray phase in radians (output).
     *                      Phase change not computed if this is NULL.
     */
    virtual void reflect_loss( 
        const wposition1& location, 
        const seq_vector& frequencies, double angle,
        vector<double>* amplitude, vector<double>* phase=NULL )
    {
        if ( location.altitude() > -236.0 ) {
            return carbonate_sand->reflect_loss(location,frequencies,angle,amplitude,phase) ;
        } else {
            return limestone->reflect_loss(location,frequencies,angle,amplitude,phase) ;
        }
    }
} ;

#endif
