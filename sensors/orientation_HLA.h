/**
 * @file orientation_HLA.h
 * Specialized version of orientation utilized in HLA.
 */
#pragma once

#include <usml/sensors/orientation.h>

namespace usml {
namespace sensors {

/**
 * Specialized version of orientation utilized in HLA.
 */
class USML_DECLSPEC orientation_HLA : public orientation {

public:

    /**
     * Empty constructor
     */
    orientation_HLA() ;

    /**
     * Constructor
     */
    orientation_HLA( double heading, double pitch, double roll ) ;

    /**
     * Destructor
     */
    virtual ~orientation_HLA() ;

protected:

    /**
     * Specialized version of apply rotation utilized in HLA.
     * Since most of the terms are zero in the computation, we can
     * ignore many of them and hence speed up the rotation computation.
     */
    virtual void apply_rotation() ;

};

}   // end of namespace sensors
}   // end of namespace usml
