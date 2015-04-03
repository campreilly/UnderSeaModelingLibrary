/** 
 * @file eigenray_collection.h
 * Container for a list of targets and their associated propagation data.
 */
#pragma once

#include <usml/ocean/ocean.h>
#include <usml/waveq3d/eigenray_listener.h>
#include <usml/waveq3d/wave_queue.h>

namespace usml {
namespace waveq3d {

using namespace usml::ocean;

/// @ingroup waveq3d
/// @{

/**
 * Container for a list of targets and their associated propagation data.
 * Passing an object of this type to a wavefront object causes it to
 * accumulates acoustic eigenrays at each location.  After propagation is
 * complete, the sum_eigenrays() method is used to collect the results
 * into a phasor-summed propagation loss and phase at each target point.
 */
class USML_DECLSPEC eigenray_collection : public eigenray_listener {

public:

	// eigenray_collection shared_ptr
	typedef boost::shared_ptr<eigenray_collection> reference;

	//typedef boost::shared_ptr<eigenray_list>  reference ;

private:

    /**
     * Matrix of target positions in world coordinates.
     */
    const wposition* _targets;

    /**
     * Frequencies over which loss was computed (Hz).
     * Linked from wavefront object so we can use it in the
     * sum_eigenrays() calculation.
     */
    const seq_vector* _frequencies;

    /**
     * Location of the wavefront source in spherical earth coordinates.
     * Linked from wavefront object so we can write it to a netCDF file.
     */
    const wposition1 _source_pos;

    /**
     * Initial depression/elevation angle at the
     * source location (degrees, positive is up).
     * Linked from wavefront object so we can write it to a netCDF file.
     */
    const seq_vector *_source_de;

    /**
     * Initial azimuthal angle at the source location
     * (degrees, clockwise from true north).
     * Ray fans that wrap around 360 deg must exclude 360 itself.
     * Linked from wavefront object so we can write it to a netCDF file.
     */
    const seq_vector *_source_az;

    /**
     * Propagation step size (seconds).
     * Linked from wavefront object so we can write it to a netCDF file.
     */
    double _time_step;

    /**
     * List of eigenrays associated with each target.
     */
    matrix< eigenray_list > _eigenrays;

    /** Total number of eigenrays.  Used by write_netcdf(). */
    int _num_eigenrays;

    /**
     * Propagation loss summed over all eigenrays.
     * Estimates of time and angle are averages weighted by
     * the amplitude in linear (non-dB) space.  The number of
     * surface bounces, bottom bounces, and caustics are taken from the
     * strongest path.  If there is no path to a particular target,
     * the number of surface bounces, bottom bounces, and caustics are all
     * set to -1.
     */
    matrix< eigenray > _loss;

public:

    /**
     * Initialize the acoustic propagation effects associated
     * with each target.
     *
     * @param   targets     Grid of targets to ensonify.
     */
    eigenray_collection(const wposition* targets);

    /**
     * Initialize with references to wave front information.
     *
     * @param	frequencies Frequencies over which to compute loss (Hz).
     * @param	source_pos  Location of the wavefront source.
     * @param	source_de   Launch D/E angle at source (deg)
     * @param	source_az   Launch AZ angle at source (deg)
     * @param	time_step   Propagation step size (seconds).
     * @param   targets     Grid of targets to ensonify.
     */
    eigenray_collection( const seq_vector& frequencies, const wposition1& source_pos,
              const seq_vector& source_de, const seq_vector& source_az,
              double time_step, const wposition* targets);

    /**
      * Destructor delete all data stored on the heap
      *
      */
    virtual ~eigenray_collection(){

        delete _frequencies;
        delete _source_de;
        delete _source_az;
    }

private:

    /**
     * Initialize with references to wave front information.
     * _loss data structure.
     */
    void initialize();

public:

    /**
     * Number of rows in target grid.
     * @return Number of rows.
     */
    inline size_t size1() const {
        return _targets->size1();
    }

    /**
     * Number of columns in target grid.
     * @return Number of columns.
     */
    inline size_t size2() const {
        return _targets->size2();
    }

    /**
     * Position of a single target in the grid.
     *
     * @param   t1          Row number of the current target.
     * @param   t2          Column number of the current target.
     * @return  wposition1  wposition1 of target.
     */
    inline wposition1 position(size_t t1, size_t t2) {
        return wposition1(*_targets, t1, t2);
    }

    /**
     * Frequencies over which propagation is computed (Hz).
     * @return  seq_vector Pointer to vector containing frequencies.
     */
    inline const seq_vector* frequencies() const {
        return _frequencies;
    }

    /**
     * Return eigenray list for a single target.
     *
     * @param   t1              Row number of the current target.
     * @param   t2              Column number of the current target.
     * @return  eigenray_list   Pointer to eigenray_list for single target.
     */
    inline eigenray_list* eigenrays(size_t t1, size_t t2) {
        return &(_eigenrays(t1, t2));
    }

    /**
     * Propagation loss for a single target summed over eigenrays.
     * Includes eigenray element weighted averages.
     *
     * @param   t1          Row number of the current target.
     * @param   t2          Column number of the current target.
     * @return  eigenray*   Pointer to eigenray class.
     */
    inline const eigenray* total(size_t t1, size_t t2) {
        return &(_loss(t1, t2));
    }

	/**
	 * add_eigenray - Adds an eigenray to the eigenray_list for the target specified.
	 * implementation of the pure virtual method of proplossListener.
	 * @param   targetRow          Row number of the current target.
	 * @param   targetCol          Column number of the current target.
	 * @param   pRay               The eigenray to add.
	 * @param   run_id             The run_id of WaveQ3D which the eigenray was produced.
	 * @return                     True on success, false on failure.
	 */
	bool add_eigenray(size_t targetRow, size_t targetCol, eigenray pRay, size_t run_id );


    /**
     * Compute propagation loss summed over all eigenrays.
     *
     * @param   coherent    Compute coherent propagation loss if true,
     *                      and incoherent if false.
     */
    void sum_eigenrays(bool coherent = true);

    /**
     * Write eigenray_collection scenario data to a netCDF file using a ragged
     * array structure. This ragged array concept (see reference) stores
     * the eigenray_collection data in a one dimensional list and uses an externally
     * defined index to lookup the appropriate elements for each target.
     *
     * This ragged array concept is used to define the intensity, phase,
     * source_de, source_az, target_de, target_az, surface, bottom, and
     * caustic variables. The proploss_index variable defines the lookup index
     * into these arrays for the summed eigenray_collection for each target.  The
     * eigenray_index variable defines a similar index for the beginning of the
     * eigenray list.  Subsequent eigenrays for this target immediately follow
     * the 1st eigenray.  The eigenray_number variable defines the number of
     * eigenrays for each target.
     *
     * The user is responsible for ensuring that sum_eigenrays() has been
     * called prior to this routine.
     *
     * This file structure is illustrated (for a single target with
     * direct path, surface, and bottom eigenrays) in the netCDF sample below:
     * <pre>
     *     netcdf eigenray_basic {
     *     dimensions:
     *     	frequency = 1 ;
     *     	rows = 1 ;
     *     	cols = 1 ;
     *     	eigenrays = 4 ;
     *     	launch_de = 25 ;
     *     	launch_az = 5 ;
     *     variables:
     *     	double source_latitude ;
     *     		source_latitude:units = "degrees_north" ;
     *     	double source_longitude ;
     *     		source_longitude:units = "degrees_east" ;
     *     	double source_altitude ;
     *     		source_altitude:units = "meters" ;
     *     		source_altitude:positive = "up" ;
     *     	double launch_de(launch_de) ;
     *     		launch_de:units = "degrees" ;
     *     		launch_de:positive = "up" ;
     *     	double launch_az(launch_az) ;
     *     		launch_az:units = "degrees_true" ;
     *     		launch_az:positive = "clockwise" ;
     *     	double time_step ;
     *     		time_step:units = "seconds" ;
     *     	double frequency(frequency) ;
     *     		frequency:units = "hertz" ;
     *     	double latitude(rows, cols) ;
     *     		latitude:units = "degrees_north" ;
     *     	double longitude(rows, cols) ;
     *     		longitude:units = "degrees_east" ;
     *     	double altitude(rows, cols) ;
     *     		altitude:units = "meters" ;
     *     		altitude:positive = "up" ;
     *     	short proploss_index(rows, cols) ;
     *     		proploss_index:units = "count" ;
     *     	short eigenray_index(rows, cols) ;
     *     		eigenray_index:units = "count" ;
     *     	short eigenray_num(rows, cols) ;
     *     		eigenray_num:units = "count" ;
     *     	double intensity(eigenrays, frequency) ;
     *     		intensity:units = "dB" ;
     *     	double phase(eigenrays, frequency) ;
     *     		phase:units = "radians" ;
     *     	double travel_time(eigenrays) ;
     *     		travel_time:units = "seconds" ;
     *     	double source_de(eigenrays) ;
     *     		source_de:units = "degrees" ;
     *     		source_de:positive = "up" ;
     *     	double source_az(eigenrays) ;
     *     		source_az:units = "degrees_true" ;
     *     		source_az:positive = "clockwise" ;
     *     	double target_de(eigenrays) ;
     *     		target_de:units = "degrees" ;
     *     		target_de:positive = "up" ;
     *     	double target_az(eigenrays) ;
     *     		target_az:units = "degrees_true" ;
     *     		target_az:positive = "clockwise" ;
     *     	short surface(eigenrays) ;
     *     		surface:units = "count" ;
     *     	short bottom(eigenrays) ;
     *     		bottom:units = "count" ;
     *     	short caustic(eigenrays) ;
     *     		caustic:units = "count" ;
     *     
     *     // global attributes:
     *     		:long_name = "eigenray_basic test" ;
     *     		:Conventions = "COARDS" ;
     *     data:
     *      source_latitude = 45 ;
     *      source_longitude = -45 ;
     *      source_altitude = -1000 ;
     *      launch_de = -60, -55, -50, -45, -40, -35, -30, -25, -20, -15, -10, -5, 0, 5, 
     *         10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60 ;
     *      launch_az = -2, -1, 0, 1, 2 ;
     *      time_step = 0.1 ;
     *      frequency = 100000 ;
     *      latitude = 45.02 ;
     *      longitude = -45 ;
     *      altitude = -1000 ;
     *      proploss_index = 0 ;
     *      eigenray_index = 1 ;
     *      eigenray_num = 3 ;
     *      intensity = 64.1427331557955, 66.9694875322853, 69.5617366839523, 77.9679676720843 ;
     *      phase = 0.082693193293971, 0, -3.14159265358979, 0 ;
     *      travel_time = 1.88788322659623, 1.48401881064351, 1.99622641373789, 3.03542140949814 ;
     *      source_de = 6.86829906648868, -0.00988717967364023, 41.9270555318522, -61.0113452826929 ;
     *      source_az = 0, 0, 0, 0 ;
     *      target_de = -6.86819212315001, 0.0101128206074365, -41.9270291816411, 61.0112432784064 ;
     *      target_az = 0, 0, 0, 0 ;
     *      surface = 0, 0, 1, 0 ;
     *      bottom = 0, 0, 0, 1 ;
     *      caustic = 0, 0, 0, 0 ;
     *     }
     * </pre>
     * @param   filename    Name of the file to write to disk.
     * @param   long_name   Optional global attribute for identifying data-set.
     *
     * @xref "The NetCDF Users Guide - Data Model, Programming Interfaces,
     * and Format for Self-Describing, Portable Data NetCDF", Version 3.6.3,
     * Section 3.4, 7 June 2008.
     */
    void write_netcdf(
            const char* filename, const char* long_name = NULL);

};

/// @}
} // end of namespace waveq3d
} // end of namespace usml
