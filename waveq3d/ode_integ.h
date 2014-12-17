/** 
 * @file ode_integ.h
 * Integration utilities for ordinary differential equations.
 */
#pragma once

#include <usml/waveq3d/wave_queue.h>

namespace usml {
namespace waveq3d {

using namespace usml::ocean ;
class wave_queue ;      // forward reference for friend declaration

/**
 * @internal
 * Integration utilities for ordinary differental equations.
 */
class USML_DECLSPEC ode_integ {

    friend class wave_queue ;
    friend class reflection_model ;
    
  private:
  
    /**
     * First position estimate in 3rd order Runge-Kutta.
     *
     * @param  dt       Time step
     * @param  y0       Initial position of wavefront (input)
     * @param  y1       First position estimate (result).
     * @param  no_alias Use uBLAS noalias() assignment speed-up if true.
     */        
    static void rk1_pos(
        double dt, wave_front *y0, wave_front *y1, bool no_alias=true ) ;
    
    /**
     * First ndirection estimate in 3rd order Runge-Kutta.
     *
     * @param  dt       Time step
     * @param  y0       Initial ndirection of wavefront (input)
     * @param  y1       First ndirection estimate (result).
     * @param  no_alias Use uBLAS noalias() assignment speed-up if true.
     */        
    static void rk1_ndir( double dt, wave_front *y0, wave_front *y1, 
        bool no_alias=true ) ;
    
    /**
     * Second position estimate in 3rd order Runge-Kutta.
     *
     * @param  dt       Time step
     * @param  y0       Initial position of wavefront (input)
     * @param  y1       First position estimate (input).
     * @param  y2       Second position estimate (result).
     * @param  no_alias Use uBLAS noalias() assignment speed-up if true.
     */        
    static void rk2_pos( double dt, wave_front *y0, wave_front *y1, 
        wave_front *y2, bool no_alias=true ) ;

    /**
     * Second ndirection estimate in 3rd order Runge-Kutta.
     *
     * @param  dt       Time step
     * @param  y0       Initial ndirection of wavefront (input)
     * @param  y1       First ndirection estimate (input).
     * @param  y2       Second ndirection estimate (result).
     * @param  no_alias Use uBLAS noalias() assignment speed-up if true.
     */        
    static void rk2_ndir( double dt, wave_front *y0, wave_front *y1, 
        wave_front *y2, bool no_alias=true ) ;

    /**
     * Third (and final) position estimate in 3rd order Runge-Kutta.
     *
     * @param  dt       Time step
     * @param  y0       Initial position of wavefront (input)
     * @param  y1       First position estimate (input).
     * @param  y2       Second position estimate (input).
     * @param  y3       Third position estimate (result).
     * @param  no_alias Use uBLAS noalias() assignment speed-up if true.
     */        
    static void rk3_pos( double dt, wave_front *y0, wave_front *y1, 
        wave_front *y2, wave_front *y3, bool no_alias=true ) ;

    /**
     * Third (and final) ndirection estimate in 3rd order Runge-Kutta.
     *
     * @param  dt       Time step
     * @param  y0       Initial ndirection of wavefront (input)
     * @param  y1       First ndirection estimate (input).
     * @param  y2       Second ndirection estimate (input).
     * @param  y3       Third ndirection estimate (result).
     * @param  no_alias Use uBLAS noalias() assignment speed-up if true.
     */        
    static void rk3_ndir( double dt, wave_front *y0, wave_front *y1, 
        wave_front *y2, wave_front *y3, bool no_alias=true ) ;

    /**
     * Adams-Bashforth (3rd order) estimate of position.
     * Includes calculation of distance between current
     * and new positions.
     *
     * @param  dt       Time step
     * @param  y0       Position of wavefront 2 iterations ago (input).
     * @param  y1       Position of wavefront 1 iteration ago (input).
     * @param  y2       Current position estimate (input).
     * @param  y3       New position estimate (result).
     * @param  no_alias Use uBLAS noalias() assignment speed-up if true.
     */        
    static void ab3_pos( double dt, wave_front *y0, wave_front *y1, 
        wave_front *y2, wave_front *y3, bool no_alias=true ) ;
        
    /**
     * Adams-Bashforth (3rd order) estimate of ndirection.
     *
     * @param  dt       Time step
     * @param  y0       Direction of wavefront 2 iterations ago (input).
     * @param  y1       Direction of wavefront 1 iteration ago (input).
     * @param  y2       Current ndirection estimate (input).
     * @param  y3       New ndirection estimate (result).
     * @param  no_alias Use uBLAS noalias() assignment speed-up if true.
     */        
    static void ab3_ndir( double dt, wave_front *y0, wave_front *y1, 
        wave_front *y2, wave_front *y3, bool no_alias=true ) ;
} ;

}  // end of namespace waveq3d
}  // end of namespace usml
