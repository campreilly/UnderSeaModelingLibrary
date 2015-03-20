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

using boost::shared_ptr ;
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
 */
class USML_DECLSPEC ocean_shared {

private:

    /** 
     * Shared reference to the current ocean. Defined as null
     * reference if ocean has not yet been defined using update().
     */
    static shared_ptr<ocean_model> _current ;

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

public:

    /**
     * Pass a shared reference of current ocean back to client.
     * Returns a null reference if ocean has not yet been 
     * defined using update().
     */
    static shared_ptr<ocean_model> current() ;

    /**
     * Update shared ocean with new data.  The shared ocean will 
     * automatically delete the ocean when no clients refer to it.
     */
    static void update( ocean_model* ocean ) ;
};

/// @}
}  // end of namespace ocean
}  // end of namespace usml
