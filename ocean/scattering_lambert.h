/**
 * @file scattering_lambert.h
 */

#ifndef USML_OCEAN_SCATTERING_LAMBERT_H
#define USML_OCEAN_SCATTERING_LAMBERT_H

#include <usml/ublas/ublas.h>
#include <usml/types/types.h>
#include <usml/ocean/scattering_model.h>

namespace usml {
namespace ocean {

using namespace usml::ublas ;
using namespace usml::types ;

using boost::numeric::ublas::vector;

class USML_DECLSPEC scattering_lambert :
        public scattering_model {

    public:

        virtual void scattering_strength( const wposition1& location,
            const seq_vector& frequencies, double angleI, double angleS,
            double azI, double azS, vector<double>* amplitude,
            vector<double>* phase ) ;

};

}   // end of namespace ocean
}   // end of namespace usml

#endif
