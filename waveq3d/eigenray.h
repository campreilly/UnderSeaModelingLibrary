/** 
 * @file eigenray.h
 * A single acoustic path between a source and target.
 */
#pragma once

#include <usml/types/types.h>
#include <list>

namespace usml {
namespace waveq3d {

using namespace usml::types ;

using boost::numeric::ublas::vector;

/// @ingroup waveq3d
/// @{

/**
 * A single acoustic path between a source and target.
 */
struct eigenray {

    ~eigenray() {
		// TODO
        //delete frequencies;
    }

    /** 
     * Time of arrival for this acoustic path (sec).
     */
    double time ;

    /**
     * Frequencies over which propagation was computed (Hz).
     */
    const seq_vector* frequencies ;
    
    /** 
     * Propagation loss as a function of frequency (dB,positive).
     */
    vector< double > intensity ;

    /** 
     * Phase change as a function of frequency (radians).
     */
    vector< double > phase ;

    /** 
     * Initial depression/elevation angle at the 
     * source location (degrees, positive is up).
     */
    double source_de ;
    
    /** 
     * Initial azimuthal angle at the source location 
     * (degrees, clockwise from true north).
     */
    double source_az ;
    
    /** 
     * Final depression/elevation angle at the 
     * target location (degrees, positive is up).
     */
    double target_de ;
    
    /** 
     * Final azimuthal angle at the target location 
     * (degrees, clockwise from true north).
     */
    double target_az ;
    
    /**
     * Number of surface reflections encountered along this path.
     */
    int surface ;
    
    /**
     * Number of bottom reflections encountered along this path.
     */
    int bottom ;

    /**
     * Number of caustics encountered along this path.
     */
    int caustic ;
};

/**
 * List of acoustic paths between a source and target.
 */
typedef std::list< eigenray > eigenray_list ;

/// @}
}  // end of namespace waveq3d
}  // end of namespace usml
