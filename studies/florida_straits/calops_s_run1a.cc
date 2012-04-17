/**
 * @example calops_s_run1a.cc
 */
#include <fstream>
#include <usml/waveq3d/waveq3d.h>
#include "reflect_loss_florida.h"

using namespace usml::waveq3d ;

/**
 * Uses WaveQ3D to predict the transmission loss for Run 1A of the calibration
 * operations test (CALOPS) experiments conducted in the Florida Straits in
 * Sept 2007 (late summer). Several papers have recently focused on the
 * presence of strong 3-D propagation effects in this area. In this study,
 * the CALOPS experimental results that were previously studied by Heaney,
 * Ballard, et. al. will be used to test the accuracy of the
 * Wavefront Queue 3-D (WaveQ3D) model.
 *
 * The bathymetry is derived from the U.S. Coastal Relief Model (CRM) on the
 * the Geophysical Data System (GEODAS) Search and Data Retrieval web site.
 * CRM provides gridded bathymetry, at 3 arc-second resolution, for the
 * U.S. East and West Coasts, the northern coast of the Gulf of Mexico,
 * Puerto Rico, and Hawaii, reaching out to the continental slope.
 *
 * The bottom loss is derived from Ballard's analysis of this area.
 * Below the 236 m isobaths, the bottom is bare limestone because loose
 * sediments have been scoured off by the Florida Current.  At shallower
 * depths, carbonate sand sediments layers cover the bottom.
 *
 * @xref K. D. Heaney and J. J. Murray, "Measurements of three-dimensional
 * propagation in a continental shelf environment," J. Acoust. Soc. Am.,
 * vol. 3, no. 125, pp. 1394-1402, March 2009.
 * @xref K. D. Heaney, R. L. Campbell, J. J. Murray, Comparison of
 * hybrid three-dimensional modeling with measurements on the
 * continental shelf, J. Acoust. Soc. Am. 131 (2), Pt. 2, February 2012
 * @xref M.S. Ballard, Modeling three-dimensional propagation in a
 * continental shelf environment, J. Acoust. Soc. Am. 131 (3), March 2012.
 * @xref Geophysical Data System (GEODAS) Search and Data Retrieval web site
 * http://www.ngdc.noaa.gov/mgg/gdas/gd_designagrid.html .
 */
int main( int argc, char* argv[] ) {
    cout << "=== calops_s_run1a ===" << endl ;
    // const char* name = "calops_s_run1a.csv" ;
    // std::ofstream os(name) ;
    // cout << "writing tables to " << name << endl ;

    // characterize the bottom using CRM bathymetry and
    // bottom loss derived from Ballard's analysis

    ascii_arc_bathy* bathymetry =
        new ascii_arc_bathy( STUDIES_FLORIDA_STRAITS_BATHYMETRY ) ;

    boundary_grid<float,2>* bottom = new boundary_grid<float,2>(
        bathymetry,
        new reflect_loss_florida( bathymetry ) ) ;

    // use simple models for sound velocity profile and surface
    // @todo upgrade SVP later

    ocean_model ocean(
        new boundary_flat(),
        bottom,
        new profile_linear() ) ;

    // define a single receiver location
    // estimate height of acoustic center 5 cm off the bottom

    wposition1 receiver( 26.0217, -79.99054 ) ;
    double rho ;
    bottom->height( receiver, &rho ) ;
    receiver.rho( rho + 0.05 ) ;
    wposition::compute_earth_radius( receiver.latitude() ) ;

    // define a series of sources locations along great circle route

    seq_linear range( 5e3, 5e3, 80e3 ) ; // 3 to 80 km
    double bearing = to_radians(8.0) ;
    wposition source( range.size(), 1, 0.0, 0.0, -100.0 ) ;
    for ( unsigned n=0 ; n < range.size() ; ++n ) {
        double d = range(n) / wposition::earth_radius ;
        double lat = to_degrees( asin(
            cos( receiver.theta() ) * cos(d) +
            sin( receiver.theta() ) * sin(d) * cos(bearing) ) ) ;
        double lng = to_degrees( receiver.phi() + asin(
            sin(bearing) * sin(d) / sin(receiver.theta()) ) ) ;
        source.latitude(  n, 0, lat ) ;
        source.longitude( n, 0, lng ) ;
        // cout << lat << "," << lng << endl ;
    }
    proploss loss( &source ) ;

    // initialize ray fan parameters

    static double f[] = { 24.0, 52.5, 106.0, 206.0, 415.0 } ;
    seq_data freq( f, 5 ) ;
    seq_rayfan de( -20.0, 20.0, 45 ) ;
    seq_linear az( -40.0, 5.0, 10.0 ) ;
    const double time_max = 60.0 ;
    const double time_step = 0.100 ;
    wave_queue wave( ocean, freq, receiver, de, az, time_step ) ;

    // propagate wavefront

    wave.init_netcdf( "wavefront.nc" ) ;
    wave.save_netcdf() ;
    while ( wave.time() < time_max ) {
        cout << "time=" << wave.time() << endl ;
        wave.step() ;
        wave.save_netcdf() ;
    }
    wave.close_netcdf() ;
    cout << "wave propagated for " << wave.time() << " secs" << endl ;

    return 0 ;
}

