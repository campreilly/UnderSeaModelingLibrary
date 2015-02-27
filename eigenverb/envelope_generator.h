///////////////////////////////////////////////////////////
//  envelope_generator.h
//  Implementation of the Class envelope_generator
//  Created on:      26-Feb-2015 5:39:17 PM
//  Original author: Ted Burns, AEgis Technologies
///////////////////////////////////////////////////////////

#include <usml/usml_config.h>

namespace usml {
namespace eigebverb {

/// @ingroup eigebverb
/// @{

class USML_DECLSPEC envelope_generator
{

public:
	envelope_generator();
	virtual ~envelope_generator();

	/**
	 * 
	 * @param DuplicateParam_1
	 */
	compute_envelopes(DuplicateParam_1);
	/**
	 * 
	 * @param DuplicateParam_1
	 */
	compute_envelopes(DuplicateParam_1);

};

/// @}
} // end of namespace eigebverb
} // end of namespace usml