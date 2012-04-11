/**
 * @example reflect_loss_florida.cc
 */
#include <usml/ocean/ocean.h>
#include <fstream>

using namespace usml::ocean;

/**
 * Compute the reflection loss for limestone and carbonate sand
 * bottoms using USML's Rayleigh model. Plots of the results
 * will be used to validate that model against Ballard's results.
 */
int main( int argc, char* argv[] ) {
    cout << "=== reflect_loss_florida ===" << endl ;
    const char* name = "reflect_loss_florida.csv" ;
    std::ofstream os(name) ;
    cout << "writing tables to " << name << endl ;

    // simple values for points and distance

    wposition1 points ;
    points.altitude(-1000.0) ;

    seq_log freq( 52.5, 1.0, 1 ) ;
    cout << "freq:  " << freq << endl ;
    vector<double> amplitude( freq.size() ) ;

    // CALOPS Summer 2007

    static double density[]   = {   2.40,   1.70 } ;
    static double speed[]     = { 3000.0, 1676.0 } ;
    static double atten[]     = {    0.1,    0.8 } ;
    static double shear[]     = { 1430.0,    0.0 } ;
    static double att_shear[] = {    0.2,    0.0 } ;

    os << "angle,limestone,sand" << endl ;
    for ( int angle = 0 ; angle <= 90 ; ++angle ) {
        os << angle ;
        for ( int n=0 ; n < 2 ; ++n ) {
            reflect_loss_rayleigh model( density[n], speed[n]/1500.0, atten[n],
                shear[n]/1500.0, att_shear[n] ) ;
            model.reflect_loss(
                points, freq, to_radians(90-angle), &amplitude ) ;
            os << "," << amplitude(0) ;
        }
        os << endl ;
    }

    return 0 ;
}
