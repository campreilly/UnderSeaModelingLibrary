/*
 * @file envelope_listener.h
 * Abstract interface for alerting listeners to the results of
 * a reverberation envelope calculation.
 */
#pragma once

#include <usml/eigenverb/envelope_collection.h>
#include <usml/threads/smart_ptr.h>

namespace usml {
namespace eigenverb {

using namespace usml::threads;

/// @ingroup eigenverb
/// @{

/**
 * Abstract interface for alerting listeners to the results of
 * a reverberation envelope calculation.
 */
class USML_DECLSPEC envelope_listener {
public:

	/**
	 * Destructor.
	 */
	virtual ~envelope_listener() {
	}

	/**
	 * Pure virtual method to update the envelopes for the object that implements it.
	 *  @param  envelopes - Shared pointer to a envelope_collection object which contains envelopes.
	 */
	virtual void update_envelopes(
			envelope_collection::reference& envelopes) = 0;

protected:

	/**
	 * Constructor - protected
	 */
	envelope_listener() {
	}

};

/// @}
}// end of namespace eigenverb
} // end of namespace usml

