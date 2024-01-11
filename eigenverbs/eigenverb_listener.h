/**
 * @file eigenverb_listener.h
 * Abstract interface for alerting listeners to the
 * results of a reverberation eigenverb calculation.
 */
#pragma once

#include <usml/eigenverbs/eigenverb_model.h>
#include <usml/usml_config.h>

#include <cstddef>

namespace usml {
namespace eigenverbs {

/// @ingroup eigenverbs
/// @{

/**
 * Abstract interface for alerting listeners to the results of
 * a reverberation eigenverb calculation.
 */
class USML_DECLSPEC eigenverb_listener {
   public:
    /**
     * Virtual destructor.
     */
    virtual ~eigenverb_listener() {}

    /**
     * Pure virtual method to add an eigenverb to the collections that holds it.
     *
     * @param verb          Eigenverb data to add to list of eigenverbs.
     * @param interface_num Interface number for the interface that generated
     *                      for this eigenverb.  See the eigenverb_collection
     *                      class header for documentation on interpreting
     *                      this number. For some layers, you can also use the
     *                      eigenverb::interface_type.
     */
    virtual void add_eigenverb(eigenverb_model::csptr verb,
                               size_t interface_num) = 0;
};

/// @}
}  // end of namespace eigenverbs
}  // end of namespace usml
