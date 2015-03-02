///////////////////////////////////////////////////////////
//  @file data_collections.h
//  Definition of the structure data_collections
//  Created on:      27-Feb-2015 5:46:40 PM
//  Original author: Ted Burns, AEgis Technologies
///////////////////////////////////////////////////////////

#pragma once

#include <boost/shared_ptr.hpp>

#include <usml/usml_config.h>
#include <usml/waveq3d/proploss.h>
#include <usml/eigenverb/envelope_collection.h>
#include <usml/eigenverb/eigenverb_collection.h>


namespace usml {
namespace eigenverb {

using namespace waveq3d;

/// @ingroup sensors
/// @{


// proploss
typedef boost::shared_ptr <proploss> proploss_shared_ptr;
// eigenverbs
typedef boost::shared_ptr <eigenverb_collection> eigenverbs_shared_ptr;
// reverb envelopes
typedef boost::shared_ptr <eigenverb_collection> envelopes_shared_ptr;


/**
 * Container for all required data of one sensor pair instance.
 */
struct USML_DECLSPEC data_collections
{
public:

    /**
     * proploss - contains targets and eigenrays 
     */
    
    proploss_shared_ptr proploss;
	
    /**
    * envelopes - contains the reverb envelopes
    */

    envelopes_shared_ptr envelopes;

    /**
    * proploss - contain all eigenverbs
    */

    eigenverbs_shared_ptr eigenverbs;

};

/// @}
} // end of namespace sensors
} // end of namespace usml
