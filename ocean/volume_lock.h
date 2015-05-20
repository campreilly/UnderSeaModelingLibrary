/**
 * @file volume_lock.h
 * A volume_model wrapper the allows access by multiple simultaneous threads.
 */
#pragma once

#include <boost/thread.hpp>
#include <usml/ocean/scattering_model.h>

namespace usml {
namespace ocean {

using boost::numeric::ublas::vector;
using boost::mutex ;
using boost::lock_guard ;

/// @ingroup boundaries
/// @{

/**
 * A volume_model wrapper the allows access by multiple simultaneous threads.
 * The depth and scattering operations have separate mutexes that allow
 * one of these to be accessed without blocking the other.
 */
class USML_DECLSPEC volume_lock : public volume_model {

public:

     /**
      * Takes control of a volume_model and creates mutex's for each public
      * method.
      */
     volume_lock(volume_model* other) : _other(other) {}

     /**
      * Deletes the volume model owned by this wrapper.
      */
     virtual ~volume_lock() {
         lock_guard<mutex> depth_guard(_depth_mutex);
         lock_guard<mutex> scattering_guard(_scattering_mutex);
         delete _other ;
     }

    //**************************************************
    // depth model

    /**
     * Compute the depth of the layer and it's thickness at
     * a series of locations.  Often used to detect collisions
     * between ray paths and the volume scattering layer.
     *
     * @param location      Location at which to compute boundary.
     * @param rho           Depth of layer center in spherical earth coords (output).
     * @param thickness     Layer thickness (output).
     */
    virtual void depth( const wposition& location,
        matrix<double>* rho, matrix<double>* thickness=NULL )
    {
        lock_guard<mutex> guard(_depth_mutex);
        _other->depth(location,rho,thickness);
    }

    /**
     * Compute the depth of the layer and it's thickness at
     * a single location.  Often used to process the reverberation
     * for individual collisions with volume scattering layer.
     *
     * @param location      Location at which to compute boundary.
     * @param rho           Depth of layer center in spherical earth coords (output).
     * @param thickness     Layer thickness (output).
     */
    virtual void depth( const wposition1& location,
        double* rho, double* thickness=NULL )
    {
        lock_guard<mutex> guard(_depth_mutex);
        _other->depth(location,rho,thickness);
    }

    //**************************************************
    // reverberation scattering strength model

    /**
     * Define a new reverberation scattering strength model.
     *
     * @param scattering    Scattering model for this layer.
     */
    void scattering( scattering_model* scattering ) {
        lock_guard<mutex> guard(_scattering_mutex);
        _other->scattering(scattering) ;
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
     * @param amplitude     Reverberation scattering strength ratio (output).
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

     /** Mutex to guard access to depth operations. */
     mutex _depth_mutex ;

     /** Mutex to guard access to scattering operations. */
     mutex _scattering_mutex ;

     /** Model that implements the volume_model behaviors. */
     volume_model* _other;
};

/// @}
}  // end of namespace ocean
}  // end of namespace usml
