/**
 * @file profile_grid.h
 * Creates a sound speed model from a 1-D, 2-D, or 3-D data grid.
 */
#pragma once

#include <usml/ocean/profile_model.h>

namespace usml {
namespace ocean {

/// @ingroup profiles
/// @{

/**
 * Sound speed model constructed from a 1-D, 2-D, or 3-D data grid.
 * The coordinate system for each kind of data set is:
 *
 *      - 1-D: Assumes that the sound speed is only a function
 *             of altitude
 *      - 2-D: Assumes that the order of axes in the grid is
 *             (altitude, latitude)
 *      - 3-D: Assumes that the order of axes in the grid is
 *             (altitude, latitude, longitude)
 *
 *    ---NOTE: altitude is the distance from the surface of the ocean
 *             to the location under the ocean with down as negative.
 *
 *    ^^^NOTE: All calculations are under the assumption that the
 *             grid axes passed in have already been transformed
 *             to their spherical earth equivalents (altitude -> rho,
 *             theta,phi).
 */
template< class DATA_TYPE, int NUM_DIMS > class profile_grid
    : public profile_model
{
  private:

    //**************************************************
    // sound speed model

    /** Sound speed for all locations. */
    data_grid<DATA_TYPE,NUM_DIMS>* _sound_speed ;

  public:

    /**
     * Compute the speed of sound and it's first derivatives at
     * a series of locations.
     *
     * @param location      Location at which to compute attenuation.
     * @param speed         Speed of sound (m/s) at each location (output).
     * @param gradient      Sound speed gradient at each location (output).
     */
    virtual void sound_speed( const wposition& location,
        matrix<double>* speed, wvector* gradient=NULL )
    {
        switch( NUM_DIMS ) {

            //***************
            // 1-D grids

            case 1 :
                if ( gradient ) {
                    matrix<double> rho( location.size1(), location.size2() ) ;
                    this->_sound_speed->interpolate(
                        location.rho(),
                        speed, &rho ) ;
                    gradient->rho( rho ) ;
                } else {
                    this->_sound_speed->interpolate(
                        location.rho(),
                        speed ) ;
                }
                break ;

            //***************
            // 2-D grids

            case 2 :
                if ( gradient ) {
                    matrix<double> rho( location.size1(), location.size2() ) ;
                    matrix<double> theta( location.size1(), location.size2() ) ;
                    this->_sound_speed->interpolate(
                        location.rho(), location.theta(),
                        speed, &rho, &theta ) ;
                    gradient->rho(rho) ;
                    gradient->theta(theta) ;
                } else {
                    this->_sound_speed->interpolate(
                        location.rho(), location.theta(),
                        speed ) ;
                }
                break ;

            //***************
            // 3-D grids

            case 3 :
                if ( gradient ) {
                    matrix<double> rho( location.size1(), location.size2() ) ;
                    matrix<double> theta( location.size1(), location.size2() ) ;
                    matrix<double> phi( location.size1(), location.size2() ) ;
                    this->_sound_speed->interpolate(
                        location.rho(), location.theta(), location.phi(),
                        speed, &rho, &theta, &phi ) ;
                    gradient->rho(rho) ;
                    gradient->theta(theta) ;
                    gradient->phi(phi) ;
                } else {
                    this->_sound_speed->interpolate(
                        location.rho(), location.theta(), location.phi(),
                        speed ) ;
                }
                break ;

            //***************
            // error

            default :
                throw std::invalid_argument(
                    "sound speed must be 1-D, 2-D, or 3-D") ;
                break ;
        }
        this->adjust_speed( location, speed, gradient ) ;
    }

    //**************************************************
    // initialization

    /**
     * Default behavior for new profile models.
     *
     * @param speed         Sound speed for the whole ocean (m/s).
     *                      Assumes control of this grid and deletes
     *                      it when the class is destroyed.
     * @param attmodel      In-water attenuation model.  Defaults to Thorp.
     *                      The profile_model takes over ownership of this
     *                      reference and deletes it as part of its destructor.
     */
    profile_grid(
        data_grid<DATA_TYPE,NUM_DIMS>* speed, attenuation_model* attmodel=NULL)
        : profile_model(attmodel), _sound_speed(speed) { }

    /**
     * Delete sound speed grid.
     */
    virtual ~profile_grid() {
        delete _sound_speed ;
    }

};

/// @}
}  // end of namespace ocean
}  // end of namespace usml
