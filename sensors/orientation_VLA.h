/**
 * @file orientation_VLA.h
 * Specialized version of orientation utilized in VLA.
 */
#pragma once

#include <usml/sensors/orientation.h>

namespace usml {
namespace sensors {

/**
 * Specialized version of orientation utilized in VLA.
 */
class USML_DECLSPEC orientation_VLA : public orientation {

public:

    /**
     * Empty constructor
     */
    orientation_VLA() ;

    /**
     * Constructor
     */
    orientation_VLA( double heading, double pitch, double roll ) ;

    /**
     * Destructor
     */
    virtual ~orientation_VLA() ;

protected:

    /**
     * Specialized version of apply rotation utilized in VLA.
     * Since most of the terms are zero in the computation, we can
     * ignore many of them and hence speed up the rotation computation.
     */
    virtual void apply_rotation() ;

};

}   // end of namespace sensors
}   // end of namespace usml
