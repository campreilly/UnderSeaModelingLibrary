///////////////////////////////////////////////////////////
//  @file envelope_collection.h
//  Implementation of the Class envelope_collection
//  Created on:      26-Feb-2015 5:39:17 PM
//  Original author: Ted Burns, AEgis Technologies
///////////////////////////////////////////////////////////

#pragma once

#include <boost/shared_ptr.hpp>

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
};

// Reverb Envelopes
typedef boost::shared_ptr <envelope_collection> envelopes_shared_ptr;

/// @}
} // end of namespace eigenverb
} // end of namespace usml

