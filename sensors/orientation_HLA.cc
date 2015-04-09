/**
 * @file orientation_HLA.cc
 */

#include <usml/sensors/orientation_HLA.h>

using namespace usml::sensors ;

/**
 * Empty constructor
 */
orientation_HLA::orientation_HLA()
    : orientation()
{

}

/**
 * Constructor
 */
orientation_HLA::orientation_HLA(
    double heading, double pitch, double roll )
    : orientation( heading, pitch, roll, scalar_vector<double>(3,0) )
{

}

/**
 * Destructor
 */
orientation_HLA::~orientation_HLA()
{

}

/**
 * Rotation computation
 */
void orientation_HLA::apply_rotation()
{
    _x = ( -cos(_pitch)*sin(_heading) + cos(_heading)*sin(_pitch)*sin(_roll) ) ;
    _y = ( cos(_heading)*cos(_pitch) + sin(_heading)*sin(_pitch)*sin(_roll) ) ;
    _z = cos(_roll)*sin(_pitch) ;
    convert_to_spherical() ;
}
