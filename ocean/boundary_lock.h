/**
 * @file boundary_lock.h
 * A wrapper for a boundary model that provides each instantiation with its own set
 * of mutex's for the height() and reflect_loss() methods.
 */
#pragma once

#include <boost/thread.hpp>
#include <usml/ocean/boundary_model.h>

namespace usml {
namespace ocean {

using boost::numeric::ublas::vector;

/// @ingroup boundaries
/// @{

/**
 * A wrapper for a boundary model that provides each instantiation with its own set
 * of mutex's for the height() and reflect_loss() methods.
 */
class USML_DECLSPEC boundary_lock : public boundary_model {

   private:

        /** Mutex's to prevent simultaneous access/update by multiple threads. */
		boost::mutex* _heightMutex ;
		boost::mutex* _reflect_lossMutex ;
		/** The "has a" object to prevent simultaneous access */
        boundary_model* _other;

    public:

        /**
         * Constructor
         * Takes control of a profile_model and creates a mutex's for each public
         * method and for each instantiation of the class and when done destroys both.
         */
        boundary_lock(boundary_model* other) : _other(other)
        {
            _heightMutex = new boost::mutex();
            _reflect_lossMutex = new boost::mutex();
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
            // Avoids try/catch on _other->height call
            boost::lock_guard<boost::mutex> heightLock(*_heightMutex);

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
            boost::lock_guard<boost::mutex> heightLock(*_heightMutex);

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
            boost::lock_guard<boost::mutex> reflect_lossLock(*_reflect_lossMutex);

            _other->reflect_loss( location, frequencies, angle, amplitude, phase ) ;
        }

        /**
         * Destructor
         */
        virtual ~boundary_lock()
        {
            if (_heightMutex || _reflect_lossMutex)
            {
                delete _heightMutex;
                delete _reflect_lossMutex;
                _heightMutex = _reflect_lossMutex = NULL;
            }
            delete _other ;
        }
};

/// @}
}  // end of namespace ocean
}  // end of namespace usml
