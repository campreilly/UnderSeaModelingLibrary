/**
 * @file boundary_mt.h
 * A wrapper for a boundary model that provides each instantiation with its own
 * mutex.
 */
#ifndef USML_OCEAN_BOUNDARY_MT_H
#define USML_OCEAN_BOUNDARY_MT_H

#include <pthreadcc.h>
#include <usml/ocean/boundary_model.h>

namespace usml {
namespace ocean {

using boost::numeric::ublas::vector;

class USML_DECLSPEC boundary_mt : public boundary_model {

   protected:

        /** Mutex to prevent simultaneous access/update by multiple threads. */
        MutualExclusion* m_pclMutex_0 ;
        MutualExclusion* m_pclMutex_1 ;

        boundary_model* _other;

    public:

        /**
         * Constructor
         * Takes control of a profile_model and creates a mutex
         * for each instantiation of the class and when done
         * destroys both.
         */
        boundary_mt(boundary_model* other) : _other(other) {
                m_pclMutex_0 = new MutualExclusion();
                m_pclMutex_1 = new MutualExclusion();
        }

        /**
         * Compute the height of the boundary and it's surface normal at
         * a series of locations.
         *
         * @param location      Location at which to compute boundary.
         * @param rho           Surface height in spherical earth coords (output).
         * @param normal        Unit normal relative to location (output).
         * @param quick_interp  Determines if you want a fast nearest or pchip interp
         */
        virtual void height( const wposition& location, matrix<double>* rho,
            wvector* normal=NULL, bool quick_interp=false ) {
                m_pclMutex_0->Lock();
//                cout << "boundary_mt::height::multi-location => Mutex is Locked" << endl;
                _other->height(location, rho, normal, quick_interp);
                m_pclMutex_0->Unlock();
//                cout << "boundary_mt::height::multi-location => Mutex is Unlocked" << endl;

        }

        /**
         * Compute the height of the boundary and it's surface normal at
         * a single location.  Often used during reflection processing.
         *
         * @param location      Location at which to compute boundary.
         * @param rho           Surface height in spherical earth coords (output).
         * @param normal        Unit normal relative to location (output).
         * @param quick_interp  Determines if you want a fast nearest or pchip interp
         */
        virtual void height( const wposition1& location, double* rho,
            wvector1* normal=NULL, bool quick_interp=false ) {
                m_pclMutex_0->Lock();
//                cout << "boundary_mt::height::single location => Mutex is Locked" << endl;
                _other->height(location, rho, normal, quick_interp);
                m_pclMutex_0->Unlock();
//                cout << "boundary_mt::height::single location => Mutex is Unlocked" << endl;

        }

        /**
         * Computes the broadband reflection loss and phase change.
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
            vector<double>* amplitude, vector<double>* phase=NULL )
        {
            m_pclMutex_1->Lock();
//            cout << "boundary_mt::reflect_loss => Mutex is Locked" << endl;
            _other->reflect_loss( location,
                frequencies, angle, amplitude, phase ) ;
            m_pclMutex_1->Unlock();
//            cout << "boundary_mt::reflect_loss => Mutex is Unlocked" << endl;
        }

        /**
         * Destructor
         */
        virtual ~boundary_mt() {
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
