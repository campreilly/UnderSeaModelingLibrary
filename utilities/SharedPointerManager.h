/**
 * @file SharedPointerManager.h
 * class that manages shared pointers for various interfaces.
 */

#ifndef USML_UTILITIES_SHAREDPOINTERMANAGER_H
#define USML_UTILITIES_SHAREDPOINTERMANAGER_H

#include <boost/shared_ptr.hpp>

namespace usml {
namespace utilities {

    template<class pointer_type>
    class SharedPointerManager {

        public:

            typedef SharedPointerManager<pointer_type>      self_type ;
            typedef boost::shared_ptr<pointer_type>          element_type ;
            typedef pointer_type *                          stored_pointer ;

            /**
             * Empty Constructor
             */
            SharedPointerManager() : __ptr( element_type() ) {}

            /**
             * Basic Constructor
             * @param ptr       a pointer to the type of shared pointer that this
             *                  class will manage.
             */
            explicit
            SharedPointerManager( stored_pointer ptr ) : __ptr( &(*ptr) ) {}

            /**
             * Destructor
             */
            virtual ~SharedPointerManager() {
                __ptr.reset() ;
            }

            /**
             * Sets the pointer that is contained within the class
             */
            void setPointer( stored_pointer ptr ) {
                __ptr.reset( &(*ptr) ) ;
            }

            virtual stored_pointer getPointer() {
                return __ptr.get() ;
            } ;

        protected:

            /**
             * The boost shared pointer.
             */
            element_type __ptr ;

    } ;

}
}

#endif
