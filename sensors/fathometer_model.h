/**
 * @file fathometer_model.h
 * Container for one fathometer_model instance.
 */
#pragma once

#include <usml/sensors/sensor_model.h>

namespace usml {
namespace sensors{

using namespace waveq3d ;
using namespace eigenverb;

/// @ingroup sensors
/// @{

/**
 * Container for one fathometer_model instance.
 * On construction takes in all source and receiver data and eigenrays
 */
class USML_DECLSPEC fathometer_model
{
public:

    /**
    * Data type used handle a collection of fathometer_model pointers.
    */
    typedef std::vector<fathometer_model*> fathometer_package;

	/**
	 * Data type used for reference to a fathometer_model.
	 */
    typedef shared_ptr<fathometer_model> reference;

    /**
     * Construct from all data required.
     *
     * @param	source_id   The source id for this pair.
     * @param	receiver_id	The receiver id for this pair.
     * @param	src_pos		The source position when eigenrays were obtained.
     * @param	rcv_pos	    The receiver position when eigenrays were obtained.
     * @param   eigenrays   Shared Pointer to the eigenray list.
     */
    fathometer_model(sensor_model::id_type source_id, sensor_model::id_type receiver_id,
                     wposition1 src_pos, wposition1 rcv_pos, shared_ptr<eigenray_list> list )
        : _source_id(source_id), _receiver_id(receiver_id),
          _source_position(src_pos), _receiver_position(rcv_pos), _eigenrays(list)
     {};

    /**
     * Destructor
     */
    ~fathometer_model()
    {
    }

    /**
     * Gets the source sensor id.
     * @return  The source sensor id.
     */
    sensor_model::id_type source_id() {
        return _source_id;
    }

    /**
     * Sets the source sensor id.
     * @param  The source sensor id
     */
    void source_id(sensor_model::id_type source_id) {
        _source_id = source_id;
    }

    /**
     * Gets the receiver sensor id.
     * @return  The receiver sensor id.
     */
    sensor_model::id_type receiver_id() {
        return _receiver_id;
    }

    /**
     * Sets the receiver sensor id.
     * @param  The receiver sensor id.
     */
    void receiver_id(sensor_model::id_type receiver_id) {
        _receiver_id = receiver_id;
    }

    /**
     * Gets the source position.
     * @return  The source position
     */
    wposition1 source_position() {
        return _source_position;
    }

    /**
     * Sets the source position
     * @param  The source position
     */
    void source_position(wposition1 position) {
        _source_position = position;
    }

    /**
     * Gets the receiver position.
     * @return  The receiver position.
     */
    wposition1 receiver_position() {
        return _receiver_position;
    }

    /**
    * Sets the receiver position.
    * @param  The receiver position
    */
    void receiver_position(wposition1 position) {
        _receiver_position = position;
    }

	/**
     * Gets the shared_ptr to last eigenray_list update for this fathometer_model.
     * @return  eigenray_list shared_ptr
     */
    boost::shared_ptr<eigenray_list> eigenrays() {
         read_lock_guard guard(_eigenrays_mutex);
         return _eigenrays;
    }

    /**
     * Sets the shared_ptr to eigenray_list for this fathometer_model.
     * @return  eigenray_list shared_ptr to the eigenray_list
     */
    void eigenrays(eigenray_list* list) {
         write_lock_guard guard(_eigenrays_mutex);
         _eigenrays = boost::shared_ptr<eigenray_list>(list);
    }

private:

    fathometer_model() {};

    /**
     * The source sensor id
     */
    sensor_model::id_type _source_id;

    /**
     * The receiver sensor id
     */
    sensor_model::id_type _receiver_id;

    /**
     * The slant range (in meters) of the sensor when the eigenrays where obtained.
     */
    double _slant_range;

    /**
     * The distance (in meters) from the sensor when the eigenrays where obtained.
     */
    double _distance_from_sensor;

    /**
     * The depth offset (in meters) from the sensor when the eigenrays where obtained.
     */
    double _depth_offset_from_sensor;

    /**
     * The position of the source sensor when the eigenrays where obtained.
     */
    wposition1 _source_position;

    /**
     * The position of the receiver sensor when the eigenrays where obtained.
     */
    wposition1 _receiver_position;

    /**
     * Eigenrays that connect source and receiver locations.
     */
    boost::shared_ptr<eigenray_list> _eigenrays;

	/**
	 * Mutex that locks during eigenray access
	 */
	mutable read_write_lock _eigenrays_mutex ;

};

/// @}
} // end of namespace sensors
} // end of namespace usml
