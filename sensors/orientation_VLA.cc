/**
 * @file orientation_VLA.cc
 */

#include <usml/sensors/orientation_VLA.h>

using namespace usml::sensors ;

/**
 * Empty constructor
 */
orientation_VLA::orientation_VLA()
    : orientation()
{

}

/**
 * Constructor
 */
orientation_VLA::orientation_VLA(
    double heading, double pitch, double roll )
    : orientation( heading, pitch, roll, scalar_vector<double>(3,0) )
{

}

/**
 * Destructor
 */
orientation_VLA::~orientation_VLA()
{

}

/**
 * Rotation computation
 */
void orientation_VLA::apply_rotation()
{
    _x = ( sin(_heading)*sin(_pitch) + cos(_heading)*cos(_pitch)*sin(_roll) ) ;
    _y = ( -cos(_heading)*sin(_pitch) + cos(_pitch)*sin(_heading)*sin(_roll) ) ;
    _z = cos(_pitch)*cos(_roll) ;
    convert_to_spherical() ;
}
