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

using boost::numeric::ublas::vector;

class USML_DECLSPEC profile_mt : public profile_model {

   protected:

        /** Mutex to prevent simultaneous access/update by multiple threads. */
        MutualExclusion* m_pclMutex_0 ;
        MutualExclusion* m_pclMutex_1 ;

        profile_model* _other;

    public:

        /**
         * Constructor
         * Takes control of a profile_model and creates a mutex
         * for each instantiation of the class and when done
         * destroys both.
         */
        profile_mt(profile_model* other) : _other(other) {
                m_pclMutex_0 = new MutualExclusion();
                m_pclMutex_1 = new MutualExclusion();
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
                m_pclMutex_0->Lock();
//                cout << "profile_mt::sound_speed => Mutex is Locked" << endl;
                _other->sound_speed(location, speed, gradient);
                m_pclMutex_0->Unlock();
//                cout << "profile_mt::sound_speed => Mutex is Unlocked" << endl;
        }

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
           matrix< vector<double> >* attenuation)
        {
            m_pclMutex_1->Lock();
           _other->attenuation(
               location, frequencies, distance, attenuation ) ;
            m_pclMutex_1->Unlock();
        }

        /**
         * Destructor
         */
        virtual ~profile_mt() {
            if (m_pclMutex_0 || m_pclMutex_1)
            {
                delete m_pclMutex_0;
                delete m_pclMutex_1;
                m_pclMutex_0 = m_pclMutex_1 = NULL;
            }
            delete _other ;
        }
};


/// @}
}// end of namespace ocean
}  // end of namespace usml

#endif
