/**
 * @example flstrts_slope_test.cc
 */
#include <fstream>
#include <usml/ocean/ocean.h>

using namespace usml::ocean;

/**
 * Compute the bottom depth and slope for the Coastal Relief Model
 * representation of the Florida Straits bathymetry->
 */
int main( int argc, char* argv[] ) {
    cout << "=== flstrts_slope_test ===" << endl ;

    ascii_arc_bathy* bathymetry = new ascii_arc_bathy( 
	USML_STUDIES_DIR "/florida_straits/flstrts_bathymetry.asc" ) ;
    boundary_grid<float,2> bottom( bathymetry ) ;

    std::ofstream file1( USML_STUDIES_DIR "/florida_straits/flstrts_slope_depth.csv" ) ;
    std::ofstream file2( USML_STUDIES_DIR "/florida_straits/flstrts_slope_normlat.csv" ) ;
    std::ofstream file3( USML_STUDIES_DIR "/florida_straits/flstrts_slope_normlng.csv" ) ;

    unsigned index[2] ;
    unsigned n=0 ;
    unsigned m=0 ;
    cout << bathymetry->axis(0)->size() << " " << bathymetry->axis(1)->size() << endl ;
    for ( n=0 ; n < bathymetry->axis(0)->size() ; ++n ) {
        for ( m=0 ; m < bathymetry->axis(1)->size() ; ++m ) {
            index[0]=n; index[1]=m;
            wposition1 location(
                to_latitude( bathymetry->axis(0)->operator()(n) ),
                to_degrees( bathymetry->axis(1)->operator()(m) ),
                0.0 ) ;
            double rho ;
            wvector1 normal ;
            bottom.height( location, &rho, &normal ) ;
            file1 << ( rho - wposition::earth_radius ) << "," ;
            file2 << normal.theta() << "," ;
            file3 << normal.phi() << "," ;
        }
        file1 << endl ;
        file2 << endl ;
        file3 << endl ;
    }

    wposition1 location( 26.0217, -79.99054, 0.0 ) ;
    double rho ;
    wvector1 normal ;
    bottom.height( location, &rho, &normal ) ;
    cout << location.latitude() << "\t" << location.longitude() << "\t" << ( rho - wposition::earth_radius )
         << "\t" << normal.theta() << "\t" << normal.phi() << endl ;

    return 0 ;
}
