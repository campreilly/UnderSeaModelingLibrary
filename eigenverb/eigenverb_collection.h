///////////////////////////////////////////////////////////
//  eigenverb_collection.h
//  Implementation of the Class eigenverb_collection
//  Created on:      26-Feb-2015 5:39:16 PM
//  Original author: Ted Burns, AEgis Technologies
///////////////////////////////////////////////////////////

#pragma once

#include <boost/shared_ptr.hpp>

#include <usml/usml_config.h>

namespace usml {
namespace eigenverb {

/// @ingroup eigenverb
/// @{

class USML_DECLSPEC eigenverb_collection
{

public:			wave_generator *_wave_generator;
	eigenverb_collection();
	virtual ~eigenverb_collection();

};

// eigenverbs
typedef boost::shared_ptr <eigenverb_collection> eigenverbs_shared_ptr;

/// @}
} // end of namespace eigenverb
} // end of namespace usml
