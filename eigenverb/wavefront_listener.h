/**
 * @file wavefront_listener.h
 * Part of the Observer/Subject pattern for the wavefront_generator class
 * which allows for multiple listeners to be added to wavefront task.
 */

#pragma once

#include <usml/waveq3d/eigenray_collection.h>
#include <usml/eigenverb/eigenverb_collection.h>
#include <usml/threads/smart_ptr.h>

namespace usml {
namespace eigenverb {


using namespace usml::waveq3d ;
using namespace usml::threads ;


/// @ingroup eigenverb
/// @{

/**
 * Part of the Observer/Subject pattern for the wavefront class
 * which allows for multiple listeners to be added to wavefront.
 * The update_eigenrays and update_eigenverbs methods must be defined in
 * each class that inherits them.
 */

class USML_DECLSPEC wavefront_listener
{
public:

    /**
     * Destructor.
     */
    virtual ~wavefront_listener() {}

    /**
     * Pure virtual method to update the eigenrays and eigenverbs for the object that implements it.
     * @param eigenrays Shared pointer to an eigenray_collection.
     * @param eigenverbs Shared pointer to an eigenverb_collection.
     */
    virtual void update_wavefront_data(eigenray_collection::reference& eigenrays,
                                        eigenverb_collection::reference& eigenverbs) = 0;
    
protected:

    /**
     * Constructor - protected
     */
    wavefront_listener() {}

private:

    // -------------------------
    // Disabling default copy constructor and default
    // assignment operator.
    // -------------------------
    wavefront_listener(const wavefront_listener& yRef);
    wavefront_listener& operator=( const wavefront_listener& yRef );

};

/// @}
} // end of namespace eigenverb
} // end of namespace usml

