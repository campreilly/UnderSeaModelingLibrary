///////////////////////////////////////////////////////////
//  @file envelope_collection.h
//  Implementation of the Class envelope_collection
//  Created on:      26-Feb-2015 5:39:17 PM
//  Original author: Ted Burns, AEgis Technologies
///////////////////////////////////////////////////////////

#pragma once

#include <usml/usml_config.h>

namespace usml {
namespace eigenverb {

/// @ingroup eigenverb
/// @{

class USML_DECLSPEC envelope_collection
{

public:
	envelope_collection();
	virtual ~envelope_collection();

	/**
	 * 
	 * @param envelopes
	 */
    void add_envelope(envelope_collection* envelopes);

};

/// @}
} // end of namespace eigenverb
} // end of namespace usml

