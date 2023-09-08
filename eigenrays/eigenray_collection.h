/**
 * @file eigenray_collection.h
 * Container for a list of targets and their associated propagation data.
 */
#pragma once

#include <usml/eigenrays/eigenray_listener.h>
#include <usml/eigenrays/eigenray_model.h>
#include <usml/ocean/profile_model.h>
#include <usml/types/seq_vector.h>
#include <usml/types/wposition.h>
#include <usml/types/wposition1.h>
#include <usml/types/wvector.h>
#include <usml/usml_config.h>

#include <boost/numeric/ublas/matrix.hpp>
#include <cstddef>
#include <memory>

namespace usml {
namespace eigenrays {

using namespace usml::ocean;

/// @ingroup eigenrays
/// @{

/**
 * Container for a list of targets and their associated propagation data.
 * Passing an object of this type to a wavefront object causes it to accumulates
 * acoustic eigenrays at each target location.  After propagation is complete,
 * the sum_eigenrays() method is used to collect the results into a
 * phasor-summed propagation loss and phase at each target point.
 */
class USML_DECLSPEC eigenray_collection : public eigenray_listener {
   public:
    /// Alias for shared reference to eigenray collection.
    typedef std::shared_ptr<const eigenray_collection> csptr;

    /**
     * Initialize with references to wave front information.
     *
     * @param frequencies   Frequencies over which to compute loss (Hz).
     * @param source_pos    Location of the wavefront source.
     * @param target_pos    Grid of targets to ensonify.
     * @param sourceID 	Value to find source in platform_manager.
     * @param targetIDs    Optional list of target IDs (default=empty).
     * @param coherent    	Compute coherent propagation totals if true.
     */
    eigenray_collection(const seq_vector::csptr &frequencies,
                        const wposition1 &source_pos,
                        const wposition &target_pos, int sourceID = 0,
                        const matrix<int> &targetIDs = matrix<int>(),
                        bool coherent = true);

    /**
     * Virtual destructor.
     */
    virtual ~eigenray_collection(){};

    /**
     * Number of rows in target grid.
     * @return Number of rows.
     */
    size_t size1() const { return _target_pos.size1(); }

    /**
     * Number of columns in target grid.
     * @return Number of columns.
     */
    size_t size2() const { return _target_pos.size2(); }

    /**
     * Position of a single target in the grid.
     *
     * @param   t1  		Row number of target.
     * @param   t2  		Column number of target.
     * @return  Position of this target.
     */
    wposition1 position(size_t t1 = 0, size_t t2 = 0) const {
        return wposition1(_target_pos, t1, t2);
    }

    /// Platform ID number for this source. Set to zero if unknown.
    int sourceID() const { return _sourceID; }

    /**
     * Platform ID number for one target. Set to zero if unknown.
     *
     * @param   t1  		Row number of target.
     * @param   t2  		Column number of target.
     * @return  Value to find target in platform_manager.
     */
    int targetID(size_t t1 = 0, size_t t2 = 0) const {
        if (size1() == 0 && size2() == 0) {
            return 0;
        }
        return _targetIDs(t1, t2);
    }

    /// Frequencies over which propagation is computed (Hz).
    seq_vector::csptr frequencies() const { return _frequencies; }

    /**
     * Return eigenray list for a single target.
     *
     * @param   t1  			Row number of target.
     * @param   t2  			Column number of target.
     * @return  Eigenray list for this target.
     */
    const eigenray_list &eigenrays(size_t t1 = 0, size_t t2 = 0) const {
        return _eigenrays(t1, t2);
    }

    /// The time of arrival of the fastest eigenray for each target.
    double initial_time(size_t t1 = 0, size_t t2 = 0) const {
        return _initial_time(t1, t2);
    }

    /**
     * Find eigenrays for a single target in the grid.
     *
     * @param   targetID	  Platform ID number for this target.
     * @return  List of acoustic paths between a source and target.
     */
    eigenray_list find_eigenrays(int targetID = 0) const;

    /**
     * Find fastest eigenray for a single target in the grid.
     *
     * @param   targetID	  Platform ID number for this target.
     * @return  Time of arrival of the fastest eigenray, zero if not found.
     */
    double find_initial_time(int targetID = 0) const;

    /**
     * Propagation loss for a single target summed over eigenrays.
     * Includes eigenray element weighted averages.
     *
     * @param   t1 			Row number of target.
     * @param   t2 			Column number of target.
     * @return  Copy of eigenray total for this target.
     */
    const eigenray_model &total(size_t t1, size_t t2) const {
        return _total(t1, t2);
    }

    /// Compute coherent propagation totals if true, and incoherent if false.
    bool coherent() const { return _coherent; }

    /**
     * Notifies the observer that a wave front collision has been detected for
     * one of the targets. Targets are specified by a row and column number.
     * Must be overloaded by sub-classes.
     *
     * @param   t1     Row number of target.
     * @param   t2     Column number of target.
     * @param   ray    Propagation loss information for this collision.
     * @param   runID  Identification number of the wavefront that
     *                 produced this result.
     * @see wave_queue.runID()
     */
    void add_eigenray(size_t t1, size_t t2, eigenray_model::csptr ray,
                      size_t runID = 0);
    /**
     * Compute propagation loss summed over all eigenrays.
     */
    void sum_eigenrays();

    /**
     * Write eigenray_collection scenario data to a netCDF file using a ragged
     * array structure. This ragged array concept (see xref) stores
     * the eigenray_collection data in a one dimensional list and uses an
     * externally defined index to lookup the appropriate elements for each
     * target.
     *
     * This ragged array concept is used to define the intensity, phase,
     * source_de, source_az, target_de, target_az, surface, bottom, and
     * caustic variables. The proploss_index variable defines the lookup index
     * into these arrays for the summed eigenray_collection for each target. The
     * eigenray_index variable defines a similar index for the beginning of the
     * eigenray list.  Subsequent eigenrays for this target immediately follow
     * the 1st eigenray.  The eigenray_number variable defines the number of
     * eigenrays for each target.
     *
     * The user is responsible for ensuring that sum_eigenrays() has been
     * called prior to this routine.
     **
     * This file structure is illustrated (for a single target with
     * direct path, surface, and bottom eigenrays) in the netCDF sample below:
     * <pre>
     *     netcdf eigenray_basic {
     *     dimensions:
     *         frequency = 1 ;
     *         rows = 1 ;
     *         cols = 1 ;
     *         eigenrays = 4 ;
     *         launch_de = 25 ;
     *         launch_az = 5 ;
     *     variables:
     *         double source_latitude ;
     *             source_latitude:units = "degrees_north" ;
     *         double source_longitude ;
     *             source_longitude:units = "degrees_east" ;
     *         double source_altitude ;
     *             source_altitude:units = "meters" ;
     *             source_altitude:positive = "up" ;
     *         double launch_de(launch_de) ;
     *             launch_de:units = "degrees" ;
     *             launch_de:positive = "up" ;
     *         double launch_az(launch_az) ;
     *             launch_az:units = "degrees_true" ;
     *             launch_az:positive = "clockwise" ;
     *         double time_step ;
     *             time_step:units = "seconds" ;
     *         double frequency(frequency) ;
     *             frequency:units = "hertz" ;
     *         double latitude(rows, cols) ;
     *             latitude:units = "degrees_north" ;
     *         double longitude(rows, cols) ;
     *             longitude:units = "degrees_east" ;
     *         double altitude(rows, cols) ;
     *             altitude:units = "meters" ;
     *             altitude:positive = "up" ;
     *         short proploss_index(rows, cols) ;
     *             proploss_index:units = "count" ;
     *         short eigenray_index(rows, cols) ;
     *             eigenray_index:units = "count" ;
     *         short eigenray_num(rows, cols) ;
     *             eigenray_num:units = "count" ;
     *         double intensity(eigenrays, frequency) ;
     *             intensity:units = "dB" ;
     *         double phase(eigenrays, frequency) ;
     *             phase:units = "radians" ;
     *         double travel_time(eigenrays) ;
     *             travel_time:units = "seconds" ;
     *         double source_de(eigenrays) ;
     *             source_de:units = "degrees" ;
     *             source_de:positive = "up" ;
     *         double source_az(eigenrays) ;
     *             source_az:units = "degrees_true" ;
     *             source_az:positive = "clockwise" ;
     *         double target_de(eigenrays) ;
     *             target_de:units = "degrees" ;
     *             target_de:positive = "up" ;
     *         double target_az(eigenrays) ;
     *             target_az:units = "degrees_true" ;
     *             target_az:positive = "clockwise" ;
     *         short surface(eigenrays) ;
     *             surface:units = "count" ;
     *         short bottom(eigenrays) ;
     *             bottom:units = "count" ;
     *         short caustic(eigenrays) ;
     *             caustic:units = "count" ;
     *
     *     // global attributes:
     *             :long_name = "eigenray_basic test" ;
     *             :Conventions = "COARDS" ;
     *     data:
     *      source_latitude = 45 ;
     *      source_longitude = -45 ;
     *      source_altitude = -1000 ;
     *      launch_de = -60, -55, -50, -45, -40, -35, -30, -25, -20, -15, -10,
     * -5, 0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60 ; launch_az = -2,
     * -1, 0, 1, 2 ; time_step = 0.1 ; frequency = 100000 ; latitude = 45.02 ;
     *      longitude = -45 ;
     *      altitude = -1000 ;
     *      proploss_index = 0 ;
     *      eigenray_index = 1 ;
     *      eigenray_num = 3 ;
     *      intensity
     * = 64.1427331557955, 66.9694875322853, 69.5617366839523, 77.9679676720843
     * ; phase = 0.082693193293971, 0, -3.14159265358979, 0 ; travel_time
     * = 1.88788322659623, 1.48401881064351, 1.99622641373789, 3.03542140949814
     * ; source_de = 6.86829906648868, -0.00988717967364023, 41.9270555318522,
     * -61.0113452826929 ; source_az = 0, 0, 0, 0 ; target_de =
     * -6.86819212315001, 0.0101128206074365,
     * -41.9270291816411, 61.0112432784064 ; target_az = 0, 0, 0, 0 ; surface =
     * 0, 0, 1, 0 ; bottom = 0, 0, 0, 1 ; caustic = 0, 0, 0, 0 ;
     *     }
     * </pre>
     * @param filename    	Name of the file to write to disk.
     * @param long_name 	Optional global attribute for identifying
     *data-set.
     *
     * @xref "The NetCDF Users Guide - Data Model, Programming Interfaces,
     * and Format for Self-Describing, Portable Data NetCDF", Version 3.6.3,
     * Section 3.4, 7 June 2008.
     */
    void write_netcdf(const char *filename,
                      const char *long_name = nullptr) const;

    /**
     * Adjust eigenrays for small changes in source/target geometry. Adjusts the
     * travel time and intensity using the component of position change along
     * each ray path. Assumes that the changes in DE, AZ, and other ray
     * components are small for small changes in position. Implemented as a two
     * step process where first the eigenrays are adjusted for changes in source
     * position, then updated again for changes in target position.
     *
     * @param t1     		Row number of target in eigenray list.
     * @param t2     		Column number of target in eigenraylist.
     * @param source_new 	Updated source location.
     * @param target_new 	Updated target location.
     * @param profile       Ocean profile model used to extract sound speed.
     *
     * @xref E. K. Skarsoullis, "Multi-section matched-peak tomographic
     * inversion with a moving source", J. Acoust. Soc. Am.
     * Vol 110, No. 2, Aug 2001.
     */
    eigenray_list dead_reckon(size_t t1, size_t t2,
                              const wposition1 &source_new,
                              const wposition1 &target_new,
                              const profile_model::csptr &profile) const;

   private:
    /// Value to find source in platform_manager. Set to zero if unknown.
    const int _sourceID;

    /// Value to find targets in platform_manager. Set to zero if unknown.
    matrix<int> _targetIDs;

    /**
     * Location of the wavefront source in spherical earth coordinates.
     * Linked from wavefront object so we can write it to a netCDF file.
     */
    const wposition1 _source_pos;

    /// Matrix of target positions in world coordinates.
    const wposition _target_pos;

    /**
     * Frequencies over which loss was computed (Hz).
     * Linked from wavefront object so we can use it in the
     * sum_eigenrays() calculation.
     */
    const seq_vector::csptr _frequencies;

    /// List of eigenrays associated with each target.
    matrix<eigenray_list> _eigenrays;

    /// The time of arrival of the fastest eigenray for each target.
    matrix<double> _initial_time;

    /// Total number of eigenrays.  Used by write_netcdf().
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
    matrix<eigenray_model> _total;

    /// Compute coherent propagation totals if true, and incoherent if false.
    bool _coherent;

    /**
     * Adjust eigenrays for small changes in the geometry of a single sensor.
     * Adjusts the travel time and intensity using the component of position
     * change along each ray path. Assumes that the changes in DE, AZ, and
     * other ray components small for small changes in position.
     *
     * Based on Equation 11 and Figure 1 in the E. K. Skarsoullis reference.
     *
     * @param eigenrays List of acoustic paths to update.
     * @param oldpos    Original sensor location.
     * @param newpos    Updated sensor location.
     * @param profile   Ocean profile model used to extract sound speed.
     *
     * @xref E. K. Skarsoullis, "Multi-section matched-peak tomographic
     * inversion with a moving source", J. Acoust. Soc. Am.
     * Vol 110, No. 2, Aug 2001.
     */
    static eigenray_list dead_reckon_one(const eigenray_list &eigenrays,
                                         const wposition1 &oldpos,
                                         const wposition1 &newpos,
                                         const profile_model::csptr &profile);
};

/// @}
}  // namespace eigenrays
}  // namespace usml
