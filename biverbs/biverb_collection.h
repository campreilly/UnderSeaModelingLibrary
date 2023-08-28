/**
 * @file biverb_collection.h
 * Collection of biverbs in the form of a vector of biverbs_lists.
 */
#pragma once

#include <usml/biverbs/biverb_model.h>
#include <usml/eigenverbs/eigenverb_model.h>
#include <usml/threads/read_write_lock.h>
#include <usml/usml_config.h>

#include <cstddef>
#include <map>
#include <memory>
#include <vector>

namespace usml {
namespace biverbs {

using namespace usml::eigenverbs;
using namespace usml::types;
using namespace usml::threads;

/// @ingroup biverbs
/// @{

/**
 * Collection of biverbs where each index represents a different interface.
 *
 *    - index=0 is eigenverbs for the bottom.
 *    - index=1 is eigenverbs for the surface.
 *    - index=2 is for the upper interface of the first
 *      volume scattering layer, if it exists.
 *    - index=3 is for the lower interface of the first
 *      volume scattering layer, if it exists.
 *    - Subsequent columns provide the upper and lower
 *      interfaces for additional volume scattering layers.
 */
class USML_DECLSPEC biverb_collection {
   public:
    /// Shared const pointer to an biverb _collection.
    typedef std::shared_ptr<const biverb_collection> csptr;

    /// Map of biverbs, sorted by time.
    typedef std::map<double, biverb_model::csptr> map;

    /**
     * Construct a collection for a series of interfaces. Creates a minimum
     * of interfaces (index 0=bottom, 1=surface), plus two for each
     * volume scattering layer.
     *
     * @param num_volumes    Number of volume scattering layers in the ocean.
     */
    biverb_collection(size_t num_volumes=0)
        : _collection((1 + num_volumes) * 2) {}

    /**
     * Number of interfaces in this collection.
     */
    size_t num_interfaces() const { return _collection.size(); }

    /**
     * Number of biverbs for a specific interface.
     *
     * @param interface Interface number of the desired list of biverbs.
     *                  See the class header for documentation on interpreting
     *                  this number.
     */
    size_t size(size_t interface) const {
        return _collection[interface].size();
    }

    /**
     * Creates list of biverbs for a specific interface.
     *
     * @param interface Interface number of the desired list of biverbs.
     *                  See the class header for documentation on interpreting
     *                  this number.
     */
    biverb_list biverbs(size_t interface) const;

    /**
     * Constructs a new bistatic eigenverb and adds it to this collection. Note
     * that passing the scattering strength as an argument allows the same
     * memory to be reused by all combinations of source and receiver eigenverb.
     *
     * @param src_verb	Source eigenverb to be processed.
     * @param rcv_verb	Receiver eigenverb to be processed.
     * @param scatter	Scattering strength vs. frequency.
     * @param interface Interface number for this addition.
     */
    void add_biverb(const eigenverb_model::csptr& src_verb,
                    const eigenverb_model::csptr& rcv_verb,
                    const vector<double>& scatter, size_t interface);

    /**
     * Writes the biverbs for an individual interface to a netcdf file.
     * There are separate variables for each biverb component,
     * and each biverb add a row to that variable.  The power,
     * length, and width variables have a column for each frequency.
     */
    void write_netcdf(const char* filename, size_t interface) const;

   private:
    /// Mutex to that locks object during changes.
    mutable read_write_lock _mutex;

    /// Spatial index for each interface.
    std::vector<map> _collection;
};

/// @}
}  // end of namespace biverbs
}  // end of namespace usml
