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
     * Constructor
     */
    sensor_pair(sensorIDType sourceID, sensorIDType receiverID)
        : _sourceID(sourceID),_receiverID(receiverID) {};

    /**
     * Destructor
     */
    ~sensor_pair()
    {

    }

    /**
     * Get the sourceID value
     * @return sourceID value
     */
    sensorIDType sourceID() {
        return _sourceID;
    }

    /**
     * Get the receiverID value
     * @return receiverID value
     */
    sensorIDType receiverID() {
        return _receiverID;
    }

    /**
     * Set the proploss shared pointer
     * @param proploss pointer
     */
    void proploss( proploss_shared_ptr proploss) {
        _proploss = proploss;
    }

    /**
     * Get the proploss shared pointer
     * @return proploss pointer
     */
    proploss_shared_ptr proploss() {
        return _proploss;
    }

    /**
     * Set the src_eigenverbs shared pointer
     * @param src_eigenverbs pointer
     */
    void src_eigenverbs( eigenverbs_shared_ptr src_eigenverbs) {
        _src_eigenverbs = src_eigenverbs;
    }

    /**
     * Get the proploss shared pointer
     * @return proploss pointer
     */
    eigenverbs_shared_ptr src_eigenverbs() {
        return _src_eigenverbs;
    }

    /**
     * Set the rcv_eigenverbs shared pointer
     * @param rcv_eigenverbs pointer
     */
    void rcv_eigenverbs( eigenverbs_shared_ptr rcv_eigenverbs) {
        _rcv_eigenverbs = rcv_eigenverbs;
    }

    /**
     * Get the rcv_eigenverbs shared pointer
     * @return rcv_eigenverbs pointer
     */
    eigenverbs_shared_ptr rcv_eigenverbs() {
        return _rcv_eigenverbs;
    }

    /**
     * Set the envelopes shared pointer
     * @param envelopes pointer
     */
    void envelopes( envelopes_shared_ptr envelopes) {
        _envelopes = envelopes;
    }

    /**
     * Get the envelopes shared pointer
     * @return envelopes pointer
     */
    envelopes_shared_ptr envelopes() {
        return _envelopes;
    }

    void update_eigenverbs(xmitRcvModeType mode, eigenverbs_shared_ptr eigenverbs)
    {

    }

private:

    sensor_pair() {};

    /**
     * sourceID - key for source.
     */
    sensorIDType _sourceID;

    /**
     * receiverID - key for receiver.
     */
    sensorIDType _receiverID;

    /**
     * proploss - contains targets and eigenrays 
     */
    proploss_shared_ptr _proploss;

    /**
     * source eigenverbs - contains all source eigenverbs
     */
    eigenverbs_shared_ptr _src_eigenverbs;

    /**
     * receiver eigenverbs - contains all receiver eigenverbs
     */
    eigenverbs_shared_ptr _rcv_eigenverbs;

    /**
     * envelopes - contains the Reverb envelopes
     */
    envelopes_shared_ptr _envelopes;

};

/// @}
} // end of namespace sensors
} // end of namespace usml
