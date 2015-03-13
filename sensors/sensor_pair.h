///////////////////////////////////////////////////////////
//  @file sensor_pair.h
//  Definition of the sensor_pair structure.
//  Created on:      27-Feb-2015 5:46:40 PM
//  Original author: Ted Burns, AEgis Technologies
///////////////////////////////////////////////////////////

#pragma once

#include <usml/usml_config.h>
#include <usml/sensors/sensorIDType.h>
#include <usml/sensors/xmitRcvModeType.h>
#include <usml/waveq3d/proploss.h>
#include <usml/eigenverb/envelope_collection.h>
#include <usml/eigenverb/eigenverb_collection.h>

namespace usml {
namespace sensors{

using namespace waveq3d ;
using namespace eigenverb;

/// @ingroup sensors
/// @{

/**
 * Container for one sensor pair instance.
 */
class USML_DECLSPEC sensor_pair
{
public:

    /**
     * sourceID - key for source.
     */
    sensorIDType sourceID;

    /**
     * receiverID - key for receiver.
     */
    sensorIDType receiverID;

    /**
     * proploss - contains targets and eigenrays 
     */
    proploss_shared_ptr proploss;

    /**
     * source eigenverbs - contains all source eigenverbs
     */
    eigenverbs_shared_ptr src_eigenverbs;

    /**
     * receiver eigenverbs - contains all receiver eigenverbs
     */
    eigenverbs_shared_ptr rcv_eigenverbs;

    /**
     * envelopes - contains the Reverb envelopes
     */
    envelopes_shared_ptr envelopes;


    void update_eigenverbs(xmitRcvModeType mode, eigenverbs_shared_ptr eigenverbs) {}

};

/// @}
} // end of namespace sensors
} // end of namespace usml
