
/**
 * @example studies/rtree/rtree_test.cc
 */
#include <iostream>
#include <fstream>
// to store queries results
#include <vector>

#include <boost/test/unit_test_suite.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <usml/eigenverb/eigenverb_collection.h>
#include <usml/netcdf/netcdf_files.h>
#include <usml/ublas/math_traits.h>

BOOST_AUTO_TEST_SUITE(rtree_test)

using namespace boost::unit_test;

using namespace usml::eigenverb ;
using namespace boost::geometry;

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

typedef bg::model::point<double, 2, bg::cs::cartesian > point;

typedef bg::model::box<point> box;

typedef std::pair<box, unsigned > value_pair;

// create the rtree using default constructor
bgi::rtree<value_pair, bgi::rstar<16,4> > rtree;

BOOST_AUTO_TEST_CASE( rtree_basic ) {
	
	cout << "=== rtree_test: rtree_basic ===" << endl;
	const char* ncname = "../usml/eigenverb/test/eigenverb_basic_";

	int interfaces = 4;
	eigenverb_collection collection(interfaces);
	eigenverb_list eigenverbs;

	// second item of "value" type in std::pair above
	unsigned  i = 0;

	// Read eigenverbs for each interface from their own disk file
	for ( int n=0 ; n < interfaces ; ++n ) {
		std::stringstream filename ;
		filename << ncname << n << ".nc" ;
		eigenverbs = collection.read_netcdf( filename.str().c_str(),n) ;

		double q = 0.0;
		double delta_lat = 0.0;
		double delta_long = 0.0;
		double latitude = 0.0;
		double longitude = 0.0;

		double lat_scaler = (60.0*1852.0); 	// meters/degree  60 nmiles/degree * 1852 meters/nmiles

		// create eigenverb values
		BOOST_FOREACH(eigenverb verb, eigenverbs)
		{
			q = sqrt(max(verb.length2[0], verb.width2[0]));

			latitude = verb.position.latitude();
			longitude = verb.position.longitude();
			delta_lat = q/lat_scaler;
			delta_long = q/(lat_scaler * cos(to_radians(latitude)));

			// create a box, first point bottom left, second point upper right
			box b(point( latitude - delta_lat, longitude - delta_long),
						point(latitude + delta_lat, longitude + delta_long));
			//std::cout << bg::wkt<box>(b) << std::endl;

			// insert value into rtree
			rtree.insert(std::make_pair(b, i));
			++i;
		}
	}

	// find values intersecting some area defined by a box
	box query_box(point(44.999, -45.005), point(45.005, -44.999));
	std::cout << "spatial query box:" << std::endl;
	std::cout << bg::wkt<box>(query_box) << std::endl;

	std::vector<value_pair> result_s;
	rtree.query(bgi::overlaps(query_box), std::back_inserter(result_s));

	// display results
	std::cout << "spatial query result:" << std::endl;
	if (result_s.size() != 0) {
		BOOST_FOREACH(value_pair const& v, result_s)
				std::cout << bg::wkt<box>(v.first) << " - " << v.second << std::endl;
		cout << " Found " << result_s.size() << " results from " << i << " eigenverbs" << endl;
	} else {
		cout << " No results found " << endl;
	}

    std::cout << "=== rtree_test: test completed! ===" << std::endl;

}

/// @}

BOOST_AUTO_TEST_SUITE_END()
