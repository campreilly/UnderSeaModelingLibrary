/**
 * @file boundary_lock.h
 * A wrapper for a boundary model that provides each instantiation with its own set
 * of mutex's for the height() and reflect_loss() and scattering methods.
 */
#pragma once

#include <boost/thread.hpp>
#include <usml/ocean/boundary_model.h>

namespace usml {
namespace ocean {

using boost::numeric::ublas::vector;
using boost::mutex ;
using boost::lock_guard ;

/// @ingroup boundaries
/// @{

/**
 * A wrapper for a boundary model that provides each instantiation with its own set
 * of mutex's for the height() and reflect_loss() methods.
 */
class USML_DECLSPEC boundary_lock : public boundary_model {

public:

    /**
     * Constructor
     * Takes control of a profile_model and creates a mutex's for each public
     * method and for each instantiation of the class and when done destroys both.
     */
    boundary_lock(boundary_model* other) : _other(other) {}

    /**
     * Destructor
     */
    virtual ~boundary_lock()
    {
        lock_guard<mutex> height_guard(_height_mutex);
        lock_guard<mutex> reflect_loss_guard(_reflect_loss_mutex);
        lock_guard<mutex> scattering_guard(_scattering_mutex);
        delete _other ;
    }

    /**
     * Compute the height of the boundary and it's surface normal at
     * a series of locations with a mutex lock.
     *
     * @param location      Location at which to compute boundary.
     * @param rho           Surface height in spherical earth coords (output).
     * @param normal        Unit normal relative to location (output).
     * @param quick_interp  Determines if you want a fast nearest or pchip interp
     */
    virtual void height( const wposition& location, matrix<double>* rho,
            wvector* normal=NULL, bool quick_interp=false )
    {
        // Locks mutex then unlocks on method exit
        // Avoids try/catch on _other->height calls
        lock_guard<mutex> guard(_height_mutex);
        _other->height(location, rho, normal, quick_interp);
    }

    /**
     * Compute the height of the boundary and it's surface normal at
     * a single location with a mutex lock.  Often used during reflection processing.
     *
     * @param location      Location at which to compute boundary.
     * @param rho           Surface height in spherical earth coords (output).
     * @param normal        Unit normal relative to location (output).
     * @param quick_interp  Determines if you want a fast nearest or pchip interp
     */
    virtual void height( const wposition1& location, double* rho,
            wvector1* normal=NULL, bool quick_interp=false )
    {
        // Locks mutex then unlocks on method exit
        lock_guard<mutex> guard(_height_mutex);
        _other->height(location, rho, normal, quick_interp);
    }

    /**
     * Computes the broadband reflection loss and phase change with a mutex lock.
     *
     * @param location      Location at which to compute attenuation.
     * @param frequencies   Frequencies over which to compute loss. (Hz)
     * @param angle         Grazing angle relative to the interface (radians).
     * @param amplitude     Change in ray strength in dB (output).
     * @param phase         Change in ray phase in dB (output).
     */
    virtual void reflect_loss(
        const wposition1& location,
        const seq_vector& frequencies, double angle,
        boost::numeric::ublas::vector<double>* amplitude, boost::numeric::ublas::vector<double>* phase=NULL )
    {
        // Locks mutex then unlocks on method exit
        lock_guard<mutex> guard(_reflect_loss_mutex);
        _other->reflect_loss( location, frequencies, angle, amplitude, phase ) ;
    }

    /**
     * Computes the broadband scattering strength for a single location.
     *
     * @param location      Location at which to compute attenuation.
     * @param frequencies   Frequencies over which to compute loss. (Hz)
     * @param de_incident   Depression incident angle (radians).
     * @param de_scattered  Depression scattered angle (radians).
     * @param az_incident   Azimuthal incident angle (radians).
     * @param az_scattered  Azimuthal scattered angle (radians).
     * @param amplitude     Change in ray strength in dB (output).
     */
    virtual void scattering( const wposition1& location,
        const seq_vector& frequencies, double de_incident, double de_scattered,
        double az_incident, double az_scattered, vector<double>* amplitude )
    {
        lock_guard<mutex> guard(_scattering_mutex);
        _other->scattering( location,frequencies, de_incident, de_scattered,
                az_incident, az_scattered, amplitude ) ;
    }

    /**
     * Computes the broadband scattering strength for a collection of
     * scattering angles from a common incoming ray. Each scattering
     * has its own location, de_scattered, and az_scattered.
     * The result is a broadband reverberation scattering strength for
     * each scattering.
     *
     * @param location      Location at which to compute attenuation.
     * @param frequencies   Frequencies over which to compute loss. (Hz)
     * @param de_incident   Depression incident angle (radians).
     * @param de_scattered  Depression scattered angle (radians).
     * @param az_incident   Azimuthal incident angle (radians).
     * @param az_scattered  Azimuthal scattered angle (radians).
     * @param amplitude     Reverberation scattering strength ratio (output).
     */
    virtual void scattering( const wposition& location,
        const seq_vector& frequencies, double de_incident, matrix<double> de_scattered,
        double az_incident, matrix<double> az_scattered, matrix< vector<double> >* amplitude )
    {
        lock_guard<mutex> guard(_scattering_mutex);
        _other->scattering( location,frequencies, de_incident, de_scattered,
                az_incident, az_scattered, amplitude ) ;
    }

private:

    /** Prevent simultaneous access/update by multiple threads */

    /** Mutex to guard access to height operations. */
    mutex _height_mutex ;

    /** Mutex to guard access to reflect_loss operations. */
    mutex _reflect_loss_mutex ;

    /** Mutex to guard access to scattering operations. */
    mutex _scattering_mutex ;

    /** The "has a" object to prevent simultaneous access */
    boundary_model* _other;

};

/// @}
}  // end of namespace ocean
}  // end of namespace usml
