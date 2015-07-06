/**
 * @file eigenverb_collection.h
 * Collection of eigenverbs in the form of a vector of eigenverbs_lists.
 */
#pragma once

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <usml/threads/threads.h>
#include <usml/eigenverb/eigenverb.h>
#include <usml/eigenverb/eigenverb_listener.h>


using namespace usml::types;
using namespace boost::geometry;

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

namespace usml {
namespace eigenverb {

typedef bg::model::point<double, 2, bg::cs::cartesian > point;

typedef bg::model::box<point> box;

typedef std::pair<point, eigenverb_list::iterator> value_pair;

typedef bgi::rtree<value_pair, bgi::rstar<16,4> > rtree_type;

/**
 * Collection of eigenverbs in the form of a vector of eigenverbs_lists.
 * Each index represents a different interface.
 *
 *    - index=0 is eigenverbs for the bottom.
 *    - index=1 is eigenverbs for the surface.
 *    - index=2 is for the upper interface of the first
 *        volume scattering layer, if it exists.
 *    - index=3 is for the lower interface of the first
 *        volume scattering layer, if it exists.
 *    - Subsequent columns provide the upper and lower
 *        interfaces for additional volume scattering layers.
 */
class USML_DECLSPEC eigenverb_collection : public eigenverb_listener {

public:

    /**
     * Shared pointer reference to an eigenverb _collection.
     */
    typedef boost::shared_ptr<eigenverb_collection> reference;

    /**
     * Construct a collection for a specific scenario.  Creates a minimum
     * of interfaces (index 0=bottom, 1=surface), plus two for each
     * volume scattering layer.
     *
     * @param num_volumes    Number of volume scattering layers in the ocean.
     */
    eigenverb_collection(size_t num_volumes) :
            _rtrees((1 + num_volumes) * 2),
            _collection((1 + num_volumes) * 2)
    {
        rtrees_ready = false;
    }

    /*
     * Virtual destructor
     */
    virtual ~eigenverb_collection() {
    }

    /**
     * Number of interfaces in this collection.
     */
    size_t num_interfaces() const {
        return _collection.size();
    }

    /**
     * Provides access to eigenverbs for a specific combination
     * of azimuth and interface.
     *
     * @param interface    Interface number of the desired list of eigenverbs.
     *                     See the class header for documentation on interpreting
     *                     this number.  For some layers, you can also use the
     *                     eigenverb::interface_type.
     */
    const eigenverb_list& eigenverbs(size_t interface) const {
        return _collection[interface];
    }

    /**
     * Adds a new eigenverb to this collection.  Make a copy of the new
     * contribution and stores the copy in its collection.
     *
     * @param verb      Eigenverb to add to the eigenverb_collection.
     * @param interface_num    Interface number of the desired list of eigenverbs.
     *                     See the class header for documentation on interpreting
     *                     this number. For some layers, you can also use the
     *                     eigenverb::interface_type.
     */
    void add_eigenverb(const eigenverb& verb, size_t interface_num) {
        _collection[interface_num].push_back(verb);
    }

    /**
     * Queries the RTree for this collection of eigenverbs at the interface and the
     * spatial box specified the rcv_eigenverb.
     * Results are return via the third parameter.
     *
     * @param interface        Interface number of the desired list of eigenverbs.
     *                             See the class header for documentation on interpreting
     *                             this number. For some layers, you can also use the
     *                             eigenverb::interface_type.
     * @param eigenverb        Eigenverb which to covert to a spatial box that
     *                             is used as the query for the rtree.
     * @param result_s        This is the result set of value_pairs in and std::vector
     */
	void query_rtree(size_t interface, eigenverb verb, std::vector<value_pair>& result_s);

    /**
     * Generates the rtrees for this collection of eigenverbs.
     * The eigenverb_collection for the source eigenverbs generates rtrees one
     * for each collection interface. The rtrees are created with the dual
     * Iterator constructor, which uses the RTree Packing Algorithm
     * to provide the fastest insertion of the data and the fastest querying.
     * A std::list is first populated with a "collection_pair"'s. A
     * collection_pair is a std::pair type that consit of a point (lat, lon)
     * and the eigenverb_list::iterator of the eigenverb_collection.
     * See http://www.boost.org/doc/libs/1_58_0/libs/geometry/doc/html/geometry/spatial_indexes/introduction.html
     */
    void generate_rtrees();

    /**
     * Writes the eigenverbs for an individual interface to a netcdf file.
     * There are separate variables for each eigenverb component,
     * and each eigenverb add a row to that variable.  The power,
     * length, and width variables have a column for each frequency.
     *
     * An example of the file format is given below.
     * <pre>
     * netcdf eigenverb_basic_2 {
     * dimensions:
     *         eigenverbs = 24 ;
     *         frequency = 2 ;
     * variables:
     *         double travel_time(eigenverbs) ;
     *                 travel_time:units = "seconds" ;
     *         double frequency(frequency) ;
     *                 frequency:units = "hertz" ;
     *         double power(eigenverbs, frequency) ;
     *                 power:units = "linear" ;
     *         double length(eigenverbs, frequency) ;
     *                 length:units = "meters" ;
     *         double width(eigenverbs, frequency) ;
     *                 width:units = "meters" ;
     *         double latitude(eigenverbs) ;
     *                 latitude:units = "degrees_north" ;
     *         double longitude(eigenverbs) ;
     *                 longitude:units = "degrees_east" ;
     *         double altitude(eigenverbs) ;
     *                 altitude:units = "meters" ;
     *         double direction(eigenverbs) ;
     *                 direction:units = "degrees_true" ;
     *                 direction:positive = "clockwise" ;
     *         double grazing_angle(eigenverbs) ;
     *                 grazing_angle:units = "degrees" ;
     *                 grazing_angle:positive = "up" ;
     *         short de_index(eigenverbs) ;
     *                 de_index:units = "count" ;
     *         short az_index(eigenverbs) ;
     *                 az_index:units = "count" ;
     *         double launch_de(eigenverbs) ;
     *                 launch_de:units = "degrees" ;
     *                 launch_de:positive = "up" ;
     *         double launch_az(eigenverbs) ;
     *                 launch_az:units = "degrees_true" ;
     *                 launch_az:positive = "clockwise" ;
     *         short surface(eigenverbs) ;
     *                 surface:units = "count" ;
     *         short bottom(eigenverbs) ;
     *                 bottom:units = "count" ;
     *         short caustic(eigenverbs) ;
     *                 caustic:units = "count" ;
     *         short upper(eigenverbs) ;
     *                 upper:units = "count" ;
     *         short lower(eigenverbs) ;
     *                 lower:units = "count" ;
     *
     * // global attributes:
     *                 :long_name = "upper volume eigenverbs" ;
     *                 :layer = 1 ;
     * data:
     *  travel_time = 2.84871692946947, 2.84871692946947, 2.84871692946947,
     *     2.84871692946947, 2.84871692946947, 2.84871692946947, 2.84871692946947,
     *     2.84871692946947, 3.04632321841142, 3.04632321841142, 3.04632321841142,
     *     3.04632321841142, 3.04632321841142, 3.04632321841142, 3.04632321841142,
     *     3.04632321841142, 3.30040431394488, 3.30040431394488, 3.30040431394488,
     *     3.30040431394488, 3.30040431394488, 3.30040431394488, 3.30040431394488,
     *     3.30040431394488 ;
     *
     *  frequency = 2000, 4000 ;
     *
     *  power =
     *   7.37605571805747e-05, 7.37605571805747e-05,
     *   7.37605571805747e-05, 7.37605571805747e-05,
     *   etc...
     *
     * }
     *
     * If the interface has no eigenverbs, the file will contain only the
     * global attributes, and there will be no dimensions, variables,
     * or data.
     * </pre>
     *
     * @param filename  Filename used to store this data.
     * @param interface    Interface number of the desired list of eigenverbs.
     *                     See the class header for documentation interpreting
     *                     this number.
     */
    void write_netcdf(const char* filename, size_t interface) const;

    /**
     * Reads the eigenverbs for a single interface from a netcdf file.
     *
     * @param filename      Filename used to store this data.
     * @param interface     Interface number of the desired list of eigenverbs.
     *                      See the class header for documentation interpreting
     *                      this number.
     * @return              eigenverb_list for the interface.
     */
    eigenverb_list read_netcdf(const char* filename, size_t interface);

private:

    /**
     * Mutex to that locks eigenverb_collection during _rtree creation
     * and querys.
     */
    mutable read_write_lock _rtree_mutex ;

    /**
     * Builds a box to insert in an rtree and/or to query the rtree.
     *
     * @param  eigenverb    Eigenverb which to covert to a box.
     * @param  sigma        Integer amount to scale up the size of the box.
     */
    box build_box(eigenverb verb, float sigma = 1);

    /**
     * Boolean to determine if the rtree have all ready been generated.
     */
    bool rtrees_ready;
    /**
     * Static value for scaling latitudes for rtrees.
     */
    static double latitude_scaler;

    /**
     * RTrees - one for each interface.
     */
    std::vector<rtree_type> _rtrees;

    /**
     * Collection of eigenverbs.
     */
    std::vector<eigenverb_list> _collection;
};

}   // end of namespace waveq3d
}   // end of namespace usml
