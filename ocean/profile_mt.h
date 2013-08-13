/**
 * @file profile_mt.h
 * A wrapper for a profile model that provides each instantiation with its own
 * mutex.
 */
#ifndef USML_OCEAN_PROFILE_MT_H
#define USML_OCEAN_PROFILE_MT_H

#include <pthreadcc.h>
#include <usml/ocean/profile_model.h>

namespace usml {
namespace ocean {

class USML_DECLSPEC profile_mt : public profile_model {

   protected:

        /** Mutex to prevent simultaneous access/update by multiple threads. */
        MutualExclusion* m_pclMutex;

        profile_model* _other;

    public:

        /**
         * Constructor
         * Takes control of a profile_model and creates a mutex
         * for each instantiation of the class and when done
         * destroys both.
         */
        profile_mt(profile_model* _other) {
                m_pclMutex = new MutualExclusion();
        }

        /**
         * Compute the speed of sound and it's first derivatives at
         * a series of locations.
         *
         * @param location      Location at which to compute attenuation.
         * @param speed         Speed of sound (m/s) at each location (output).
         * @param gradient      Sound speed gradient at each location (output).
         */
        virtual void sound_speed( const wposition& location,
            matrix<double>* speed, wvector* gradient=NULL ) {
                m_pclMutex->Lock();
                _other->sound_speed(location, speed, gradient);
                m_pclMutex->Unlock();
        }

        /**
         * Destructor
         */
        virtual ~profile_mt() {
            if (m_pclMutex)
            {
                delete m_pclMutex;
                m_pclMutex = NULL;
            }
            delete _other ;
        }
};


/// @}
}// end of namespace ocean
}  // end of namespace usml

#endif
