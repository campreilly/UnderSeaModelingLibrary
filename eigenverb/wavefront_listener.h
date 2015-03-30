/*
 * @file wavefront_listener.h
 *
 *  Created on: March 25, 2015
 *      Author: Ted Burns, AEgis Technologies Group, Inc.
 */

#pragma once

#include <usml/waveq3d/proploss.h>
#include <usml/eigenverb/eigenverb_collection.h>
#include <usml/threads/smart_ptr.h>

namespace usml {
namespace eigenverb {

using namespace usml::waveq3d ;
using namespace usml::threads ;

/// @ingroup eigenverb
/// @{

/**
 * @class wavefront_listener
 * This class is part of a Observer/Subject pattern for the wavefront class
 * and allows for multiple listeners to be added to wavefront.
 * The update_fathometers and update_eigenverbs methods must be defined in
 * each class which inherits it.
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
	 *  @param  fathometers - Pointer to a proploss object which contains eigenrays
	 */
    virtual void update_fathometers(shared_ptr<proploss>& fathometers) = 0;

    /**
     * Pure virtual method to update the eigenverb_collection for the object that implements it.
     *  @param  eigenverbs - Pointer to a eigenverb_collection object which contains eigenverbs
     */
    virtual void update_eigenverbs( shared_ptr<eigenverb_collection>& eigenverbs) = 0;
	
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

