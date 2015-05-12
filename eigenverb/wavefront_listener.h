/**
 * @file wavefront_listener.h
 * Part of the Observer/Subject pattern for the wavefront class
 * which allows for multiple listeners to be added to wavefront.
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
     * Pure virtual method to update the eigenrays for the object that implements it.
     *  @param  eigenrays - Shared pointer to a eigenray_collection object which contains eigenrays.
     */
    virtual void update_eigenrays(eigenray_collection::reference& eigenrays) = 0;

    /**
     * Pure virtual method to update the eigenverb_collection for the object that implements it.
     *  @param  eigenverbs - Shared pointer to a eigenverb_collection object which contains eigenverbs.
     */
    virtual void update_eigenverbs(eigenverb_collection::reference& eigenverbs) = 0;
    
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

