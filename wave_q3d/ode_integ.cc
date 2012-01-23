/** 
 * @file ode_integ.cc
 * Integration utilities for ordinary differental equations.
 */
#include <usml/wave_q3d/ode_integ.h>

using namespace usml::wave_q3d ;

/**
 * First position estimate in 3rd order Runge-Kutta.
 */        
void ode_integ::rk1_pos(
    double dt, wave_front *y0, wave_front *y1, bool no_alias )
{
    y1->position.rho( 
        y0->position.rho() 
        + 0.5 * dt * y0->pos_gradient.rho(),
        no_alias ) ;
    y1->position.theta( 
        y0->position.theta() 
        + 0.5 * dt * y0->pos_gradient.theta(),
        no_alias ) ;
    y1->position.phi( 
        y0->position.phi() 
        + 0.5 * dt * y0->pos_gradient.phi(),
        no_alias ) ;
}

/**
 * First ndirection estimate in 3rd order Runge-Kutta.
 */        
void ode_integ::rk1_ndir( double dt, wave_front *y0, wave_front *y1, 
    bool no_alias )
{
    y1->ndirection.rho( 
        y0->ndirection.rho() 
        + 0.5 * dt * y0->ndir_gradient.rho(),
        no_alias ) ;
    y1->ndirection.theta( 
        y0->ndirection.theta()
        + 0.5 * dt * y0->ndir_gradient.theta(),
        no_alias ) ;
    y1->ndirection.phi( 
        y0->ndirection.phi()
        + 0.5 * dt * y0->ndir_gradient.phi(),
        no_alias ) ;
}

/**
 * Second position estimate in 3rd order Runge-Kutta.
 */        
void ode_integ::rk2_pos( double dt, wave_front *y0, wave_front *y1, 
    wave_front *y2, bool no_alias )
{
    y2->position.rho( 
        y0->position.rho() 
        + dt * ( 2.0 * y1->pos_gradient.rho() - y0->pos_gradient.rho() ),
        no_alias ) ;
    y2->position.theta( 
        y0->position.theta() 
        + dt * ( 2.0 * y1->pos_gradient.theta() - y0->pos_gradient.theta() ),
        no_alias ) ;
    y2->position.phi( 
        y0->position.phi()
        + dt * ( 2.0 * y1->pos_gradient.phi() - y0->pos_gradient.phi() ),
        no_alias ) ;
}

/**
 * Second ndirection estimate in 3rd order Runge-Kutta.
 */        
void ode_integ::rk2_ndir( double dt, wave_front *y0, wave_front *y1, 
    wave_front *y2, bool no_alias )
{
    y2->ndirection.rho( 
        y0->ndirection.rho()
        + dt * ( 2.0 * y1->ndir_gradient.rho() - y0->ndir_gradient.rho() ),
        no_alias ) ;
    y2->ndirection.theta( 
        y0->ndirection.theta() 
        + dt * ( 2.0 * y1->ndir_gradient.theta() - y0->ndir_gradient.theta() ),
        no_alias ) ;
    y2->ndirection.phi( 
        y0->ndirection.phi()
        + dt * ( 2.0 * y1->ndir_gradient.phi() - y0->ndir_gradient.phi() ),
        no_alias ) ;
}

/**
 * Third (and final) position estimate in 3rd order Runge-Kutta.
 */        
void ode_integ::rk3_pos( double dt, wave_front *y0, wave_front *y1, 
    wave_front *y2, wave_front *y3, bool no_alias )
{
    y3->position.rho( 
        y0->position.rho() + dt / 6.0 * 
        ( y0->pos_gradient.rho() 
        + 4.0 * y1->pos_gradient.rho() 
        + y2->pos_gradient.rho() ),
        no_alias ) ;
    y3->position.theta( 
        y0->position.theta() + dt / 6.0 * 
        ( y0->pos_gradient.theta() 
        + 4.0 * y1->pos_gradient.theta() 
        + y2->pos_gradient.theta() ),
        no_alias ) ;
    y3->position.phi( 
        y0->position.phi() + dt / 6.0 * 
        ( y0->pos_gradient.phi() 
        + 4.0 * y1->pos_gradient.phi() 
        + y2->pos_gradient.phi() ),
        no_alias ) ;
}

/**
 * Third (and final) ndirection estimate in 3rd order Runge-Kutta.
 */        
void ode_integ::rk3_ndir( double dt, wave_front *y0, wave_front *y1, 
    wave_front *y2, wave_front *y3, bool no_alias )
{
    y3->ndirection.rho( 
        y0->ndirection.rho() + dt / 6.0 * 
        ( y0->ndir_gradient.rho() 
        + 4.0 * y1->ndir_gradient.rho() 
        + y2->ndir_gradient.rho() ),
        no_alias ) ;
    y3->ndirection.theta( 
        y0->ndirection.theta() + dt / 6.0 * 
        ( y0->ndir_gradient.theta() 
        + 4.0 * y1->ndir_gradient.theta() 
        + y2->ndir_gradient.theta() ),
        no_alias ) ;
    y3->ndirection.phi( 
        y0->ndirection.phi() + dt / 6.0 * 
        ( y0->ndir_gradient.phi() 
        + 4.0 * y1->ndir_gradient.phi() 
        + y2->ndir_gradient.phi() ),
        no_alias ) ;
}

/**
 * Adams-Bashforth (3rd order) estimate of position.
 */        
void ode_integ::ab3_pos( double dt, wave_front *y0, wave_front *y1, 
    wave_front *y2, wave_front *y3, bool no_alias )
{
    static const double A2 = 23.0 / 12.0 ;
    static const double A1 = 16.0 / 12.0 ;
    static const double A0 =  5.0 / 12.0 ;
    
    y3->position.rho( dt * 
        ( A2 * y2->pos_gradient.rho() 
        - A1 * y1->pos_gradient.rho()
        + A0 * y0->pos_gradient.rho() ), no_alias ) ;
    y3->position.theta( dt * 
        ( A2 * y2->pos_gradient.theta() 
        - A1 * y1->pos_gradient.theta()
        + A0 * y0->pos_gradient.theta() ), no_alias ) ;
    y3->position.phi( dt * 
        ( A2 * y2->pos_gradient.phi() 
        - A1 * y1->pos_gradient.phi()
        + A0 * y0->pos_gradient.phi() ), no_alias ) ;

    y3->distance = sqrt(
        abs2( y3->position.rho() ) +
        abs2( element_prod( y2->position.rho(), y3->position.theta() ) ) +
        abs2( element_prod( y2->position.rho(), 
            element_prod( sin(y2->position.theta()), y3->position.phi() )
        ) )
    ) ;
    
    y3->position.rho(   y2->position.rho()   + y3->position.rho(),   false ) ;
    y3->position.theta( y2->position.theta() + y3->position.theta(), false ) ;
    y3->position.phi(   y2->position.phi()   + y3->position.phi(),   false ) ;
}

/**
 * Adams-Bashforth (3rd order) estimate of ndirection.
 */        
void ode_integ::ab3_ndir( double dt, wave_front *y0, wave_front *y1, 
    wave_front *y2, wave_front *y3, bool no_alias )
{
    static const double A2 = 23.0 / 12.0 ;
    static const double A1 = 16.0 / 12.0 ;
    static const double A0 =  5.0 / 12.0 ;
    
    y3->ndirection.rho( y2->ndirection.rho() + dt * 
        ( A2 * y2->ndir_gradient.rho() 
        - A1 * y1->ndir_gradient.rho()
        + A0 * y0->ndir_gradient.rho() ), no_alias ) ;
    y3->ndirection.theta( y2->ndirection.theta() + dt * 
        ( A2 * y2->ndir_gradient.theta() 
        - A1 * y1->ndir_gradient.theta()
        + A0 * y0->ndir_gradient.theta() ), no_alias ) ;
    y3->ndirection.phi( y2->ndirection.phi() + dt * 
        ( A2 * y2->ndir_gradient.phi() 
        - A1 * y1->ndir_gradient.phi()
        + A0 * y0->ndir_gradient.phi() ), no_alias ) ;
}
