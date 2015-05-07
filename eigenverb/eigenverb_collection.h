/*
 * @file eigenverb_collection.h
 * Collection of eigenverbs in the form of a vector of eigenverbs_lists.
 */
#pragma once

#include <usml/eigenverb/eigenverb.h>
#include <usml/eigenverb/eigenverb_listener.h>
//#include <usml/types/quadtree.h>

using namespace usml::types;

namespace usml {
namespace eigenverb {

//typedef quadtree_type<eigenverb,100>::points   eigenverb_tree ;

/**
 * Collection of eigenverbs in the form of a vector of eigenverbs_lists.
 * Each index represents a different interface.
 *
 *    - index=0 is eigenverbs for the bottom.
 *    - index=1 is eigenverbs for the surface.
 *    - index=2 is for the upper interface of the first
 *    	volume scattering layer, if it exists.
 *    - index=3 is for the lower interface of the first
 *    	volume scattering layer, if it exists.
 *    - Subsequent columns provide the upper and lower
 *    	interfaces for additional volume scattering layers.
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
	 * @param num_volume	Number of volume scattering layers in the ocean.
	 */
	eigenverb_collection(size_t num_volumes) :
			_collection((1 + num_volumes) * 2) {
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
	 * @param interface	Interface number of the desired list of eigenverbs.
	 * 					See the class header for documentation on interpreting
	 * 					this number.  For some layers, you can also use the
	 * 					eigenverb::interface_type.
	 */
	const eigenverb_list& eigenverbs(size_t interface) const {
		return _collection[interface];
	}

	/**
	 * Adds a new eigenverb to this collection.  Make a copy of the new
	 * contribution and stores the copy in its collection.
	 *
	 * @param interface	Interface number of the desired list of eigenverbs.
	 * 					See the class header for documentation on interpreting
	 * 					this number. For some layers, you can also use the
	 * 					eigenverb::interface_type.
	 */
	void add_eigenverb(const eigenverb& verb, size_t interface) {
		_collection[interface].push_back(verb);
	}

	/**
	 * Writes the eigenverbs for an individual interface to a netcdf file.
	 * There are separate variables for each eigenverb component,
	 * and each eigenverb add a row to that variable.  The energy,
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
     *         double energy(eigenverbs, frequency) ;
     *                 energy:units = "linear" ;
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
     *  energy =
     *   7.37605571805747e-05, 7.37605571805747e-05,
     *   7.37605571805747e-05, 7.37605571805747e-05,
     *   etc...
     *
     * }
	 * <pre>
	 * If the interface has no eigenverbs, the file will contain only the
	 * global attributes, and there will be no dimensions, variables,
	 * or data.
	 *
	 * @param filename  File uses to store this data.
	 * @param interface	Interface number of the desired list of eigenverbs.
	 * 					See the class header for documentation interpreting
	 * 					this number.
	 */
	void write_netcdf(const char* filename, size_t interface) const;

private:

	/**
	 * Collection of eigenverbs.
	 *
	 * TODO Use an quadtree instead of a std::list to store the eigenverbs,
	 *      We hope this will improve the speed of envelope calculations.
	 */
	std::vector<eigenverb_list> _collection;
};

}   // end of namespace waveq3d
}   // end of namespace usml
