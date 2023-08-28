/**
 * @file eigenverb_collection.h
 * Collection of eigenverbs in the form of a vector of eigenverb_lists.
 */
#pragma once

#include <usml/eigenverbs/eigenverb_listener.h>
#include <usml/eigenverbs/eigenverb_model.h>
#include <usml/threads/read_write_lock.h>
#include <usml/usml_config.h>

#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/index/parameters.hpp>
#include <boost/geometry/index/rtree.hpp>
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
 * In addition to structures for storing eigenverbs, it also includes the algorithms
 * for eigenverb searches and writing eigenverbs to disk.
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
    eigenverb_list find_eigenverbs(eigenverb_model::csptr bounding_verb,
                                   size_t interface) const;

    /**
     * Writes the eigenverbs for an individual interface to a netcdf file.
     * There are separate variables for each eigenverb component,
     * and each eigenverb add a row to that variable.  The power,
     * length, and width variables have a column for each frequency.
     *
     * An example of the file format is given below.
     * <pre>
     *	netcdf create_eigenverbs {
     *	dimensions:
     *		eigenverbs = 80 ;
     *		frequency = 1 ;
     *	variables:
     *		double travel_time(eigenverbs) ;
     *			travel_time:units = "seconds" ;
     *		double frequency(frequency) ;
     *			frequency:units = "hertz" ;
     *		double power(eigenverbs, frequency) ;
     *			power:units = "dB" ;
     *		double length(eigenverbs) ;
     *			length:units = "meters" ;
     *		double width(eigenverbs) ;
     *			width:units = "meters" ;
     *		double latitude(eigenverbs) ;
     *			latitude:units = "degrees_north" ;
     *		double longitude(eigenverbs) ;
     *			longitude:units = "degrees_east" ;
     *		double altitude(eigenverbs) ;
     *			altitude:units = "meters" ;
     *		double direction(eigenverbs) ;
     *			direction:units = "degrees_true" ;
     *			direction:positive = "clockwise" ;
     *		double grazing_angle(eigenverbs) ;
     *			grazing_angle:units = "degrees" ;
     *			grazing_angle:positive = "up" ;
     *		double sound_speed(eigenverbs) ;
     *			sound_speed:units = "m/s" ;
     *		short de_index(eigenverbs) ;
     *			de_index:units = "count" ;
     *		short az_index(eigenverbs) ;
     *			az_index:units = "count" ;
     *		double source_de(eigenverbs) ;
     *			source_de:units = "degrees" ;
     *			source_de:positive = "up" ;
     *		double source_az(eigenverbs) ;
     *			source_az:units = "degrees_true" ;
     *			source_az:positive = "clockwise" ;
     *		short surface(eigenverbs) ;
     *			surface:units = "count" ;
     *		short bottom(eigenverbs) ;
     *			bottom:units = "count" ;
     *		short caustic(eigenverbs) ;
     *			caustic:units = "count" ;
     *		short upper(eigenverbs) ;
     *			upper:units = "count" ;
     *		short lower(eigenverbs) ;
     *			lower:units = "count" ;
     *		double bounding_north(eigenverbs) ;
     *			bounding_north:units = "degrees_north" ;
     *		double bounding_south(eigenverbs) ;
     *			bounding_south:units = "degrees_north" ;
     *		double bounding_east(eigenverbs) ;
     *			bounding_east:units = "degrees_east" ;
     *		double bounding_west(eigenverbs) ;
     *			bounding_west:units = "degrees_east" ;
     *
     *	// global attributes:
     *			:long_name = "bottom eigenverbs" ;
     *	data:
     *	 travel_time = 0.67695107459, 0.67695107459, 0.70945184831, ...
     *	 frequency = 3000 ;
     *	 power = 0, 0, 0, ...
     *	 length = 89.9796820017376, 89.9796820017376, 98.8270288890316, ...
     *	 width = 15.3874318673162, 15.3874318673162, 31.7623948360016, ...
     *   latitude = 15.0015638705682, 15.0015879959731, 15.0032281064754, ...
     *   longitude = 35.0002854821935, 35, 35.0005892905853, ...
     *   altitude = -1000, -1000, -1000, ...
     *   direction = 10, 0, 10, ....
     *   grazing_angle = 80, 80, 70, ...
     *   sound_speed = 1500, 1500, 1500, ...
     *   de_index = 0, 0, 1, ...
     *   az_index = 1, 0, 1, ...
     *   source_de = -80, -80, -70, ...
     *   source_az = 10, 0, 10, ...
     *	 surface = 0, 0, 0, ...
     *	 bottom = 0, 0, 0, ...
     *	 caustic = 0, 0, 0, ...
     *	 lower = 0, 0, 0, ...
     *	 bounding_north = 15.00276004, 15.002802624, 15.004541897, ...
     *	 bounding_south = 15.00036769, 15.000373367, 15.001914315, ...
     *   bounding_east = 35.000503842, 35.000215042, 35.001026436, ...
     *   bounding_west = 35.000067122, 34.999784957, 35.000152144, ...
     *	}
     * </pre>
     * If the interface has no eigenverbs, the file will contain only the
     * global attributes, and there will be no dimensions, variables,
     * or data.
     *
     * @param filename      Filename used to store this data.
     * @param interface     Interface number for this list of eigenverbs.
     *                      See the class header for documentation interpreting
     *                      this number.
     */
    void write_netcdf(const char* filename, size_t interface) const;

    /**
     * Reads the eigenverbs for a single interface from a NetCDF file.
     *
     * @param filename      Filename used to retrive this data.
     * @param interface     Interface number for this list of eigenverbs.
     *                      See the class header for documentation interpreting
     *                      this number.
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
