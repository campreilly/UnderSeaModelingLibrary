///////////////////////////////////////////////////////////
//  @file source_data.h
//  Definition of the source_data struture.
//  Created on:      27-Feb-2015 5:46:40 PM
//  Original author: Ted Burns, AEgis Technologies
///////////////////////////////////////////////////////////

#pragma once

#include <usml/usml_config.h>
#include <usml/waveq3d/proploss.h>
#include <usml/eigenverb/eigenverb_collection.h>

namespace usml {
namespace eigenverb {

using namespace waveq3d;

/// @ingroup eigenverb
/// @{

/**
 * Container for all required source data of one sensor pair instance.
 */
struct USML_DECLSPEC source_data
{
public:

    /**
     * proploss - contains targets and eigenrays 
     */
    proploss_shared_ptr proploss;

    /**
     * eigenverbs - contains all Source eigenverbs
     */
    eigenverbs_shared_ptr eigenverbs;

};

/// @}
} // end of namespace eigenverb
} // end of namespace usml
