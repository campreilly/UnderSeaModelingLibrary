/**
 * @file ocean_shared.h
 * Shares an ocean singleton across multiple threads.
 */
#pragma once

#include <usml/ocean/ocean_model.h>
#include <usml/threads/threads.h>
#include <boost/shared_ptr.hpp>

namespace usml {
namespace ocean {

using namespace usml::threads ;

/// @ingroup ocean_model
/// @{

/**
 * Shares an ocean singleton across multiple threads. One thread uses
 * the update() method to publish a new ocean model.  Other
 * threads use the current() method to get reference to this ocean.
 * Clients get a new reference to the shared ocean before they start 
 * to propagate a new wave_queue, but use that same definition for 
 * duration of that cycle. The ocean is returned to these clients 
 * as a shared pointer so that a new ocean can be defined without 
 * blocking clients that are actively using the previous setting.  
 *
 * Uses mutex locking to control multi-threaded access to the current() and
 * update() methods.  Multiple readers can access the current() simultaneously,
 * but updating the ocean using update() blocks other readers and writers.
 *
 * Warnings:
 * - The shared ocean must be build using the _lock variants of the
 *   profile_model, boundary_model, and volume_model. These variants
 *   use mutex locking when the models are used by multiple execution threads.
 */
class USML_DECLSPEC ocean_shared {

public:

    /**
     * Defines a reference to a shared ocean.
     */
    typedef boost::shared_ptr<ocean_model> reference ;

    /**
     * Pass a shared reference of current ocean back to the client.
     * Returns a null reference if ocean has not yet been
     * defined using update().
     */
    static ocean_shared::reference current() ;

    /**
     * Update shared ocean singleton with new data.
     *
     * @param   ocean  Shared pointer to the data used to update this singleton.
     */
    static void update( ocean_shared::reference& ocean ) ;

    /**
     * Reset the shared ocean pointer to empty.
     */
    static void reset();

private:

    /** 
     * Shared reference to the current ocean. Defined as null
     * reference if ocean has not yet been defined using update().
     */
    static reference _current ;

    /** Locks singleton while ocean is being changed. */
    read_write_lock _lock ;

    /**
     * Hide constructors to prevent incorrect use of singleton.
     */
    ocean_shared() {}

    /**
     * Hide copy constructors to prevent incorrect use of singleton.
     */
    ocean_shared( const ocean_shared& ) {}

};

/// @}
}  // end of namespace ocean
}  // end of namespace usml
