/**
 * @example flstrts_btmloss_test.cc
 */
#include <fstream>
#include <usml/ocean/ocean.h>
#include "flstrts_btmloss.h"

using namespace usml::ocean;

/**
 * Compute the reflection loss for limestone and carbonate sand
 * bottoms using USML's Rayleigh model. Plots of the results
 * will be used to validate that model against Ballard's results.
 *
 * @xref M.S. Ballard, Modeling three-dimensional propagation in a
 * continental shelf environment, J. Acoust. Soc. Am. 131 (3), March 2012.
 */
int main( int argc, char* argv[] ) {
    cout << "=== flstrts_btmloss_test ===" << endl ;
    const char* name = USML_STUDIES_DIR "/florida_straits/flstrts_btmloss_test.csv" ;
    std::ofstream os(name) ;
    cout << "writing tables to " << name << endl ;

    ascii_arc_bathy bathymetry( USML_STUDIES_DIR "/florida_straits/flstrts_bathymetry.asc" ) ;

    flstrts_btmloss bottom( &bathymetry ) ;
    seq_log freq( 52.5, 1.0, 1 ) ;
    vector<double> sand( freq.size() ), limestone( freq.size() ) ;

    os << "angle,carbonate sand,limestone" << endl ;
    for ( int angle = 0 ; angle <= 90 ; ++angle ) {
        double a = to_radians(90.0-angle);
        bottom.reflect_loss( wposition1(26.5,-80), freq, a, &sand ) ;
        bottom.reflect_loss( wposition1(26.5,-79.9), freq, a, &limestone ) ;
        os << angle << "," << sand(0) << "," << limestone(0) << endl ;
    }
    return 0 ;
}
