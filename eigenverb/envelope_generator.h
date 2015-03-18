///////////////////////////////////////////////////////////
//  @file envelope_generator.h
//  Implementation of the Class envelope_generator
//  Created on:      26-Feb-2015 5:39:17 PM
//  Original author: Ted Burns, AEgis Technologies
///////////////////////////////////////////////////////////

#include <usml/usml_config.h>
#include <usml/eigenverb/envelope_collection.h>

namespace usml {
namespace eigenverb {

/// @ingroup eigenverb
/// @{

class USML_DECLSPEC envelope_generator
{
private:
	envelope_collection* _envelope_collection;

public:
	envelope_generator();
	virtual ~envelope_generator();

	/**
	 * 
	 * @param 
	 */
	void compute_envelopes();

};

/// @}
} // end of namespace eigenverb
} // end of namespace usml