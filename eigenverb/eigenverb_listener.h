/**
 * @file eigenverb_listener.h
 * Abstract interface for alerting listeners to the
 * results of a reverberation eigenverb calculation.
 */
#pragma once

#include <usml/eigenverb/eigenverb.h>
#include <usml/threads/smart_ptr.h>

namespace usml {
namespace eigenverb {

using namespace usml::threads;

/// @ingroup eigenverb
/// @{

/**
 * Abstract interface for alerting listeners to the results of
 * a reverberation eigenverb calculation.
 */
class USML_DECLSPEC eigenverb_listener {
public:

    /**
     * Destructor.
     */
    virtual ~eigenverb_listener() {
    }

    /**
     * Pure virtual method to add an eigenverb for the object that implements it.
     *  @param  verb      - eigenverb data to add to list of eigenverbs.
     *  @param  interface_num - Interface number for the interface that generated
     *                      for this eigenverb.  See the eigenverb_collection
     *                      class header for documentation on interpreting
     *                      this number. For some layers, you can also use the
     *                      eigenverb::interface_type.
     */
    virtual void add_eigenverb(const eigenverb& verb, size_t interface_num) = 0;

protected:

    /**
     * Constructor - protected
     */
    eigenverb_listener() {
    }
private:

    // -------------------------
    // Disabling default copy constructor and default
    // assignment operator.
    // -------------------------
    eigenverb_listener(const eigenverb_listener& yRef);
    eigenverb_listener& operator=(const eigenverb_listener& yRef);

};

/// @}
} // end of namespace eigenverb
} // end of namespace usml

