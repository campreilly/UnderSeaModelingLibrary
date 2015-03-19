/**
 * @file profile_lock.h
 * A wrapper for a USML profile model that provides each instantiation with its own set
 * of mutex's for the sound_speed() and attenuation() methods.
 */
#pragma once

#include <boost/thread.hpp>
#include <usml/ocean/profile_model.h>

namespace usml {
namespace ocean {

using boost::numeric::ublas::vector;

/// @ingroup profiles
/// @{

/**
 * A wrapper for a USML profile model that provides each instantiation with its own set
 * of mutex's for the sound_speed() and attenuation() methods.
 */
class USML_DECLSPEC profile_lock : public profile_model {

   private:

        /** Mutex to prevent simultaneous access/update by multiple threads. */
		boost::mutex* _sound_speedMutex ;
		boost::mutex* _attenuationMutex ;
	    /** The "has a" object to prevent simultaneous access */
        profile_model* _other;

    public:

        /**
         * Constructor
         * Takes control of a profile_model and creates mutex's fpr each public
         * method and for each instantiation of the class and when done destroys both.
         */
        profile_lock(profile_model* other) : _other(other)
        {
            _sound_speedMutex = new boost::mutex();
            _attenuationMutex = new boost::mutex();
        }

        /**
         * Compute the speed of sound and it's first derivatives at
         * a series of locations with a mutex lock.
         *
         * @param location      Location at which to compute attenuation.
         * @param speed         Speed of sound (m/s) at each location (output).
         * @param gradient      Sound speed gradient at each location (output).
         */
        virtual void sound_speed( const wposition& location,
            matrix<double>* speed, wvector* gradient=NULL )
        {
            // Locks mutex then unlocks on method exit
            // Avoids try/catch on _other->sound_speed call
            boost::lock_guard<boost::mutex> sound_speedLock(*_sound_speedMutex);

            _other->sound_speed(location, speed, gradient);

        }

        /**
        * Computes the broadband absorption loss of sea water with a mutex lock.
        *
        * @param location      Location at which to compute attenuation.
        * @param frequencies   Frequencies over which to compute loss. (Hz)
        * @param distance      Distance traveled through the water (meters).
        * @param attenuation   Absorption loss of sea water in dB (output).
        */
        virtual void attenuation(
           const usml::types::wposition& location,
           const seq_vector& frequencies,
           const boost::numeric::ublas::matrix<double>& distance,
           boost::numeric::ublas::matrix< boost::numeric::ublas::vector<double> >* attenuation)
       {
            // Locks mutex then unlocks on method exit
            // Avoids try/catch on _other->attenuation call
            boost::lock_guard<boost::mutex> attenuationLock(*_attenuationMutex);

            _other->attenuation(location, frequencies, distance, attenuation ) ;
       }

        /**
         * Destructor
         */
        virtual ~profile_lock()
        {
            if (_sound_speedMutex || _attenuationMutex)
            {
               delete _sound_speedMutex;
               delete _attenuationMutex;
               _sound_speedMutex = _attenuationMutex = NULL;
            }
            delete _other ;
        }
};

/// @}
}  // end of namespace ocean
}  // end of namespace usml
