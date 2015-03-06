/**
 * @example types/test/quadtree_test.cc
 */
#include <boost/test/unit_test.hpp>
#include <usml/types/quadtree.h>
#include <usml/types/test/quadtree_test_support.h>
#include <iostream>
#include <list>
#include <cstdlib>

BOOST_AUTO_TEST_SUITE(quadtree_test)

using namespace boost::unit_test ;
using namespace usml::types ;
using namespace std ;

/**
 * @ingroup types_test
 * Test the construction, insertion and querying of the
 * quadtree class using points.
 */
BOOST_AUTO_TEST_CASE( quadtree_points ) {

    cout << "=== quadtree_test: quadtree_points ===" << endl;

    cout << "Creating a quadtree..." ;
    quadtree_type<point,1000>::points point_tree(-5000,-5000,10000,10000) ;
    cout << "done." << endl ;

    point p ;
//    box b( 22, 71, 70, 10 ) ;
    box b( -100, -37, 45, 20 ) ;
    list<point> truth ;
    srand(1) ;
    size_t N = 1000000 ;
    cout << "Populating the quadtree..." ;
    for(size_t i=0; i<N; ++i) {
        p.x = rand() % 5000 * std::pow( -1.0, i ) ;
        p.y = rand() % 5000 * std::pow( -1.0, i ) ;
        if( b.x <= p.x && p.x <= (b.x+b.width) ) {
            if( b.y <= p.y && p.y <= (b.y+b.height) ) {
                truth.push_back( p ) ;
            }
        }
        point_tree.insert(p) ;
    }
    cout << "complete." << endl ;
    truth.sort( sort_by_x<point> ) ;
//    point_tree.print() ;

    // query test
    list<point> result ;
    cout << "Querying the quadtree..." ;
    point_tree.query( b, &result ) ;
    cout << "finished, size of list is: " << result.size() << endl ;
    result.sort( sort_by_x<point> ) ;

    BOOST_CHECK_EQUAL( truth.size(), result.size() ) ;
    bool good = compare_list(truth,result) ;
    cout << "Query was " << ( good ? "successful" : "unsuccessful" ) << endl ;
    if( !good ) {
        cout << "Result list: " << endl ;
        print_list( result ) ;
        cout << "Truth list: " << endl ;
        print_list( truth ) ;
    }
}

BOOST_AUTO_TEST_SUITE_END()
