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

class USML_DECLSPEC boundary_mt : public boundary_model {

   protected:

        /** Mutex to prevent simultaneous access/update by multiple threads. */
        MutualExclusion* m_pclMutex;

        boundary_model* _other;

    public:

        /**
         * Constructor
         * Takes control of a profile_model and creates a mutex
         * for each instantiation of the class and when done
         * destroys both.
         */
        boundary_mt(boundary_model* _other) {
                m_pclMutex = new MutualExclusion();
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
                m_pclMutex->Lock();
                _other->height(location, rho, normal, quick_interp);
                m_pclMutex->Unlock();

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
                m_pclMutex->Lock();
                _other->height(location, rho, normal, quick_interp);
                m_pclMutex->Unlock();

        }

        /**
         * Destructor
         */
        virtual ~boundary_mt() {
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
