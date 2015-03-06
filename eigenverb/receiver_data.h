///////////////////////////////////////////////////////////
//  @file receiver_data.h
//  Definition of the receiver_data structure.
//  Created on:      27-Feb-2015 5:46:40 PM
//  Original author: Ted Burns, AEgis Technologies
///////////////////////////////////////////////////////////

#pragma once

#include <usml/usml_config.h>
#include <usml/eigenverb/envelope_collection.h>
#include <usml/eigenverb/eigenverb_collection.h>

namespace usml {
namespace eigenverb {

/**
 * Container for all required receiver data of one sensor pair instance.
 */
struct USML_DECLSPEC receiver_data
{
public:

    /**
     * envelopes - contains the Reverb envelopes
     */
    envelopes_shared_ptr envelopes;

    /**
     * eigenverbs - contains all Receiver eigenverbs
     */
    eigenverbs_shared_ptr eigenverbs;

};

/// @}
} // end of namespace eigenverb
} // end of namespace usml
