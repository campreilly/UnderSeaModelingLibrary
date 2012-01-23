/* randgen.cc - UNCLASSIFIED */

#include <usml/ublas/randgen.h>
#include <ctime>

using usml::ublas::randgen ;

/**
* Master generator that underlies all distributions.
* Seeded with the current system time at initialization.
*/
randgen::random_type randgen::master_gen(time(0));

/**
* Uniform random number generator for the range [0,1).
*/
boost::uniform_01<randgen::random_type,double> 
    randgen::uniform_gen( randgen::master_gen ) ;

/** 
 * variate_generator<> requires reference to abstract distribution. 
 */
boost::normal_distribution<double> randgen::gaussian_dist(0.0,1.0) ;

/**
* Gaussian ransom number generator with sigma = 1.
*/
boost::variate_generator< randgen::random_type, 
    boost::normal_distribution<double> > randgen::gaussian_gen(
	randgen::master_gen, randgen::gaussian_dist ) ;

