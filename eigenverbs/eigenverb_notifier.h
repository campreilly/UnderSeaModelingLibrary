/**
 * @file eigenverb_notifier.h
 * Manages eigenverb listeners and distributes eigenverb updates.
 */
#pragma once

#include <usml/eigenverbs/eigenverb_listener.h>
#include <usml/eigenverbs/eigenverb_model.h>
#include <usml/usml_config.h>

#include <cstddef>
#include <set>

namespace usml {
namespace eigenverbs {

/// @ingroup eigenverbs
/// @{

/**
 * Manages eigenverb listeners and distributes eigenverb updates.
 */
class USML_DECLSPEC eigenverb_notifier {
   public:
    /**
     * Add an eigenverb listener to this object.
     */
    void add_eigenverb_listener(eigenverb_listener* listener);

    /**
     * Remove an eigenverb listener to this object.
     */
    void remove_eigenverb_listener(eigenverb_listener* listener);

    /**
     * Distribute an eigenverb update to all listeners.
     *
     * @param verb          Eigenverb that defines area for query.
     * @param interface_num Interface number for this query.
     *                      See the eigenverb class header for documentation
     *                      on interpreting this number.
     */
    void notify_eigenverb_listeners(const eigenverb_model::csptr& verb,
                                    size_t interface_num) const;

    /**
     * Determines if any listeners exist
     * @return true when listeners exist, false otherwise.
     */
    bool has_eigenverb_listeners() const { return _listeners.size() > 0; }

   private:
    /**
     * List of active eigenverb listeners.
     */
    std::set<eigenverb_listener*> _listeners;
};

/// @}
}  // end of namespace eigenverbs
}  // end of namespace usml
