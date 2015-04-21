/**
 * @file beam_pattern_multi.h
 */
#pragma once

#include <usml/sensors/beam_pattern_model.h>
#include <list>

namespace usml {
namespace sensors {

class USML_DECLSPEC beam_pattern_multi
   : public beam_pattern_model
{

public:

    /**
     * Constructor
     * Takes a list of beam patterns and stores them locally to be used
     * when requesting a beam level.
     */
    beam_pattern_multi( std::list<beam_pattern_model*> beam_list ) ;

    /**
     * Destructor
     */
    virtual ~beam_pattern_multi() ;

    /**
     * Computes the response level in a specific DE/AZ pair and
     * beam steering angle. The return, level, is passed
     * back in linear units.
     *
     * @param de            Depression/Elevation angle (rad)
     * @param az            Azimuthal angle (rad)
     * @param orient        Orientation of the array
     * @param frequencies   List of frequencies to compute beam level for
     * @param level         Beam level for each frequency (linear units)
     */
    virtual void beam_level( double de, double az,
                             orientation& orient,
                             const vector<double>& frequencies,
                             vector<double>* level) ;

    /**
     * Computes the directivity index for a list of frequencies
     *
     * @param frequencies   frequencies to determine DI at
     * @param level         gain for the provided frequencies
     */
    virtual void directivity_index( const vector<double>& frequencies,
                                    vector<double>* level ) ;

private:

    /**
     * List of beam patterns associated with this multi-pattern
     */
    std::list<beam_pattern_model*>  _beam_list ;

};

}   // end of namespace sensors
}   // end of namespace usml
