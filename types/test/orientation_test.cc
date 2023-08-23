/**
 * @example types/test/orientation_test.cc
 */
#include <usml/types/orientation.h>

#include <boost/test/unit_test.hpp>
#include <boost/numeric/ublas/assignment.hpp>
#include <iostream>     // std::cout, std::fixed
#include <iomanip>      // std::setprecision


BOOST_AUTO_TEST_SUITE(orientation_test)

using namespace boost::unit_test;
using namespace usml::types;

/** Tolerance for value comparisons */
const double tol = 1e-10 ;

/**
 * pretty print boost matrix
 */
void pretty_print_matrix(const matrix<double> &m)
{
    for(unsigned i=0;i<m.size1();++i)
    {
        cout<<"| ";
        for (unsigned j=0;j<m.size2();++j)
        {
            cout<< std::fixed << std::setprecision(2) << m(i,j)<<" | ";
        }
        cout<<"|"<<endl;
    }
    cout << std::scientific;
}

/**
 * @ingroup orientationation_test
 * @{
 */

/**
 * Test that yaw rotates the vehicle in the clockwise direction from north.
 */
BOOST_AUTO_TEST_CASE(just_yaw) {
	cout << "=== just_yaw ===" << endl ;

	orientation body(45.0, 0.0, 0.0) ;
	vector<double> front = body.front();
	vector<double> right = body.right();
	vector<double> up = body.up();
	cout << "front=" << front << " right=" << right << " up=" << up << endl ;

	// check that forward points north-east
	
	const double value = cos(to_radians(45.0)) ;
    BOOST_CHECK_CLOSE( front(0), value, tol ) ;
    BOOST_CHECK_CLOSE( front(1), value, tol ) ;
    BOOST_CHECK_SMALL( front(2), tol ) ;

	// check that right points south-east
	
    BOOST_CHECK_CLOSE( right(0), -value, tol ) ;
    BOOST_CHECK_CLOSE( right(1), value, tol ) ;
    BOOST_CHECK_SMALL( right(2), tol ) ;

	// check that up is unchanged

    BOOST_CHECK_SMALL( up(0), tol ) ;
    BOOST_CHECK_SMALL( up(1), tol ) ;
    BOOST_CHECK_CLOSE( up(2), 1.0, tol ) ;
}

/**
 * Test that pitch rotates the nose of the vehicle up.
 */
BOOST_AUTO_TEST_CASE(just_pitch) {
	cout << "=== just_pitch ===" << endl ;

	orientation body(0.0, 45.0, 0.0) ;
	vector<double> front = body.front();
	vector<double> right = body.right();
	vector<double> up = body.up();
	cout << "front=" << front << " right=" << right << " up=" << up << endl ;

	// check that forward points north and up

	const double value = cos(to_radians(45.0)) ;
    BOOST_CHECK_CLOSE( front(0), value, tol ) ;
    BOOST_CHECK_SMALL( front(1), tol ) ;
    BOOST_CHECK_CLOSE( front(2), value, tol ) ;

	// check that right is unchanged

    BOOST_CHECK_SMALL( right(0), tol ) ;
    BOOST_CHECK_CLOSE( right(1), 1.0, tol ) ;
    BOOST_CHECK_SMALL( right(2), tol ) ;

	// check that up points south and up

    BOOST_CHECK_CLOSE( up(0), -value, tol ) ;
    BOOST_CHECK_SMALL( up(1), tol ) ;
    BOOST_CHECK_CLOSE( up(2), value, tol ) ;
}

/**
 * Test that roll drops the right side down while lifting the left side.
 */
BOOST_AUTO_TEST_CASE(just_roll) {
	cout << "=== just_roll ===" << endl ;

	orientation body(0.0, 0.0, 45.0) ;
	vector<double> front = body.front();
	vector<double> right = body.right();
	vector<double> up = body.up();
	cout << "front=" << front << " right=" << right << " up=" << up << endl ;

	// check that forward is unchanged

	const double value = cos(to_radians(45.0)) ;
    BOOST_CHECK_CLOSE( front(0), 1.0, tol ) ;
    BOOST_CHECK_SMALL( front(1), tol ) ;
    BOOST_CHECK_SMALL( front(2), tol ) ;

	// check that right is tipped down

    BOOST_CHECK_SMALL( right(0), tol ) ;
    BOOST_CHECK_CLOSE( right(1), value, tol ) ;
    BOOST_CHECK_CLOSE( right(2), -value, tol ) ;

	// check that up is tipped east

    BOOST_CHECK_SMALL( up(0), tol ) ;
    BOOST_CHECK_CLOSE( up(1), value, tol ) ;
    BOOST_CHECK_CLOSE( up(2), value, tol ) ;
}

/**
 * Test combination of rotations in all three dimensions.
 * First, we rotation in yaw by 45 degrees so that the axes become
 * front=(0.707,0.707,0) right=(-0.707,0.707,0) up=(0,0,1).
 * Next, we rotate up by 45 degrees.  The right axes does change, but the up
 * component of the front axis increases, while the up axis shifts into the
 * negative part of the north and east directions such that
 * front=(0.5,0.5,0.707) right=(-0.707,0.707,0) up=[3](-0.5,-0.5,0.707).
 * Finally, we roll to the right by 90 degrees.  The front axis doesn't change,
 * but the new up axis equals the old right axis, and the new right axis is the
 * negative of the old up axis such that
 * front=(0.5,0.5,0.707) right=[3](0.5,0.5,-0.707) up=[3](-0.707,0.707,0).
 */
BOOST_AUTO_TEST_CASE(rotate_all) {
	cout << "=== rotate_all ===" << endl ;

	orientation body(45.0, 45.0, 90.0) ;
	vector<double> front = body.front();
	vector<double> right = body.right();
	vector<double> up = body.up();
	cout << "front=" << front << " right=" << right << " up=" << up << endl ;

	// check that forward points north, east, and up

	const double value = cos(to_radians(45.0)) ;
	const double value2 = value  * value ;
    BOOST_CHECK_CLOSE( front(0), value2, tol ) ;
    BOOST_CHECK_CLOSE( front(1), value2, tol ) ;
    BOOST_CHECK_CLOSE( front(2), value, tol ) ;

	// check that right points north, east, and down

    BOOST_CHECK_CLOSE( right(0), value2, tol ) ;
    BOOST_CHECK_CLOSE( right(1), value2, tol ) ;
    BOOST_CHECK_CLOSE( right(2), -value, tol ) ;

	// check that up points south, east, and parallel to horizon

    BOOST_CHECK_CLOSE( up(0), -value, tol ) ;
    BOOST_CHECK_CLOSE( up(1), value, tol ) ;
    BOOST_CHECK_SMALL( up(2), tol ) ;
}

/**
 * Test the ability to combine orientations and extract yaw, pitch, roll from
 * the resulting rotation matrix.
 */
BOOST_AUTO_TEST_CASE(combine_orientations) {
	cout << "=== combine_orientations ===" << endl ;

	orientation body(45.0, 0.0, 0.0) ;
	orientation sensor_body(0.0, 45.0, 90.0) ;
	orientation sensor_world ;
	sensor_world.rotate(body, sensor_body) ;

	vector<double> front = sensor_world.front();
	vector<double> right = sensor_world.right();
	vector<double> up = sensor_world.up();
	cout << "front=" << front << " right=" << right << " up=" << up << endl ;

	// check that forward points north, east, and up

	const double value = cos(to_radians(45.0)) ;
	const double value2 = value  * value ;
    BOOST_CHECK_CLOSE( front(0), value2, tol ) ;
    BOOST_CHECK_CLOSE( front(1), value2, tol ) ;
    BOOST_CHECK_CLOSE( front(2), value, tol ) ;

	// check that right points north, east, and down

    BOOST_CHECK_CLOSE( right(0), value2, tol ) ;
    BOOST_CHECK_CLOSE( right(1), value2, tol ) ;
    BOOST_CHECK_CLOSE( right(2), -value, tol ) ;

	// check that up points south, east, and parallel to horizon

    BOOST_CHECK_CLOSE( up(0), -value, tol ) ;
    BOOST_CHECK_CLOSE( up(1), value, tol ) ;
    BOOST_CHECK_SMALL( up(2), tol ) ;

    // check ability to extract yaw, pitch, and roll

    BOOST_CHECK_CLOSE(sensor_world.yaw(),45.0,tol);
    BOOST_CHECK_CLOSE(sensor_world.pitch(),45.0,tol);
    BOOST_CHECK_CLOSE(sensor_world.roll(),90.0,tol);
}


/**
 * Test the built in vector rotation function
 *
 * Test that we can take a vector in world coordates and rotate it into local
 * coordinates.
 *
 * We will reuse a test case from above of yaw 45, pitch 45, roll 90 which
 * produces the following body unit vectors in world coordaites:
 *  front=(0.5,0.5,0.707) right=[3](0.5,0.5,-0.707) up=[3](-0.707,0.707,0).
 *
 * We will create unit vectors that existed in the wolrd system and rotate
 * those into the local coordinate system.  Expected result is:
 * north=(0.5,0.5,-0.707) east=(0.5,0.5,0.707) up=(0.707107,-0.707,0.0)
 *
 */
BOOST_AUTO_TEST_CASE(rotate_vector) {
	cout << "=== rotate_vector ===" << endl ;
	orientation body(45.0, 45.0, 90.0) ;
	vector<double> front = body.front();
	vector<double> right = body.right();
	vector<double> up = body.up();
	cout << "body units in world coordinates" << endl;
	cout << "front=" << front << " right=" << right << " up=" << up << endl ;

	cout << "rotation matrix is" << endl;
	pretty_print_matrix(body.rotation());

	/* create unit vectors in the world cooridates*/
	matrix<double> world_units (3,3);
	world_units <<=	1.0, 0.0, 0.0,
					0.0, 1.0, 0.0,
					0.0, 0.0, 1.0;
	cout << "wolrd unit vectors" << endl;
	pretty_print_matrix(world_units);

	/* rotate these into the body's system */
	body.rotate_vector(world_units);
	cout << "wolrd unit vector rotated" << endl;
	pretty_print_matrix(world_units);

	/* what we think the output should be */
	matrix<double> world_units_expected (3,3);
	world_units_expected <<=	0.5, 			0.5, 			sqrt(2.0)/2.0,
								0.5, 			0.5, 			-sqrt(2.0)/2.0,
								-sqrt(2.0)/2.0,	sqrt(2.0)/2.0, 	0.0;
	cout << "wolrd unit expected" << endl;
	pretty_print_matrix(world_units_expected);

	for(int i=0; i<3; i++){
		for(int j=0; j<3; j++){
			BOOST_CHECK_SMALL(
					world_units(i,j) -
					world_units_expected(i,j),
					tol
					);
		}
	}

}

/// @}

BOOST_AUTO_TEST_SUITE_END()
