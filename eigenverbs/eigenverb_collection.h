/**
 * @file eigenverb_collection.h
 * Collection of eigenverbs in the form of a vector of eigenverb_lists.
 */
#pragma once

#include <usml/eigenverbs/eigenverb_listener.h>
#include <usml/eigenverbs/eigenverb_model.h>
#include <usml/threads/read_write_lock.h>
#include <usml/usml_config.h>

#pragma GCC diagnostic push

// clang-17 doesn't issue any warnings about "geometry.hpp", and
// also it doesn't support "-Wmaybe-uninitialized".
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif

#include <boost/geometry/geometry.hpp>
#pragma GCC diagnostic pop

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

namespace usml {
namespace eigenverbs {

using namespace usml::types;
using namespace usml::threads;
namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;
namespace bgm = boost::geometry::model;

/// @ingroup eigenverbs
/// @{

/**
 * Collection of eigenverbs where each index represents a different interface.
 *
 *    - index=0 is eigenverbs for the bottom.
 *    - index=1 is eigenverbs for the surface.
 *    - index=2 is for the upper interface of the first
 *      volume scattering layer, if it exists.
 *    - index=3 is for the lower interface of the first
 *      volume scattering layer, if it exists.
 *    - Subsequent columns provide the upper and lower
 *      interfaces for additional volume scattering layers.
 *
 * In addition to structures for storing eigenverbs, it also includes the
 * algorithms for eigenverb searches and writing eigenverbs to disk.
 */
class USML_DECLSPEC eigenverb_collection : public eigenverb_listener {
   public:
    /// Shared const pointer to an eigenverb collection.
    typedef std::shared_ptr<const eigenverb_collection> csptr;

    /**
     * Scale factor for size of search area in find_eigenverbs(). Defaults to
     * a 3.0 value.
     */
    static double search_scale;

    /**
     * Construct a collection for a series of interfaces. Creates a minimum
     * of interfaces (index 0=bottom, 1=surface), plus two for each
     * volume scattering layer.
     *
     * @param num_volumes    Number of volume scattering layers in the ocean.
     */
    eigenverb_collection(size_t num_volumes = 0)
        : _collection((1 + num_volumes) * 2) {}

    /**
     * Number of interfaces in this collection.
     */
    size_t num_interfaces() const { return _collection.size(); }

    /**
     * Number of eigenverbs for a specific interface.
     *
     * @param interface Interface number of the desired list of eigenverbs.
     */
    size_t size(size_t interface) const {
        return _collection[interface].size();
    }

    /**
     * Creates list of eigenverbs for a specific interface.
     *
     * @param interface Interface number of the desired list of eigenverbs.
     */
    eigenverb_list eigenverbs(size_t interface) const;

    /**
     * Adds a new eigenverb to this collection.
     *
     * @param verb      Eigenverb reference to add to the eigenverb_collection.
     * @param interface Interface number for this addition.
     */
    void add_eigenverb(eigenverb_model::csptr verb, size_t interface);

    /**
     * Finds all of the eigenverbs near another eigenverb. Computes a ploygonal
     * search area that is roughly 3 times as big as the major and minor axes
     * for the bounding_verb. Then uses an rtree search to find all the
     * eigenverbs whose positions are inside that search area.
     *
     * @param bounding_verb		Eigenverb that defines bounding box.
     * @param interface 		Interface number for this query.
     * @return          		List for eigenverbs that overlap this
     * area.
     */
    eigenverb_list find_eigenverbs(const eigenverb_model::csptr& bounding_verb,
                                   size_t interface) const;

    /**
     * Writes the eigenverbs for an individual interface to a netcdf file. There
     * are separate variables for each eigenverb component, and each eigenverb
     * adds a row to that variable.  The power variable also has a column
     * for frequencies. All other variables are independent of frequency,
     * If the interface has no eigenverbs, the file will contain only the
     * global attributes, and there will be no dimensions, variables,
     * or data.
     *
     * @param filename      Filename used to store this data.
     * @param interface     Interface number for this list of eigenverbs.
     */
    void write_netcdf(const char* filename, size_t interface) const;

    /**
     * Reads the eigenverbs for a single interface from a NetCDF file.
     *
     * @param filename      Filename used to retrive this data.
     * @param interface     Interface number for this list of eigenverbs.
     */
    void read_netcdf(const char* filename, size_t interface);

   private:
    /// Point in geographic coordinates, based on degrees.
    typedef bgm::point<double, 2, bg::cs::spherical_equatorial<bg::degree>>
        point;

    /// Eigenverb paired with its geographic coordinate.
    typedef std::pair<point, eigenverb_model::csptr> pair;

    /// Spatial index for eigenverbs in geographic coordinates.
    typedef bgi::rtree<pair, bgi::rstar<8>> rtree;

    /// Mutex to that locks object during changes.
    mutable read_write_lock _mutex;

    /// Spatial index for each interface.
    std::vector<rtree> _collection;
};

/// @}
}  // end of namespace eigenverbs
}  // end of namespace usml
