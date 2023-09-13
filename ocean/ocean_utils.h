/**
 * @file ocean_utils.h
 * Ocean generation utilities.
 */
#pragma once

#include <usml/ocean/reflect_loss_rayleigh.h>

namespace usml {
namespace ocean {

/// @ingroup ocean_model
/// @{

/**
 * Ocean generation utilities.
 */
class ocean_utils {
   public:
    // Hide constructor.
    ocean_utils() = delete;

    /**
     * Creates a isovelocity ocean with no absorption and a flat bottom.
     *
     * @param depth			Ocean depth.
     * @param bottom_loss	Bottom reflection amplitude change (dB).
     */
    static void make_iso(double depth, double bottom_loss = 0.0);

    /**
     * Creates a simple, but realistic, ocean from the databases delivered with
     * USML. Uses ETOPO1 database for bathymetry and World Ocean Atlas (WOA) for
     * sound speed profile. Uses Eckart model for surface loss and Rayleigh for
     * bottom loss. Stores the result in the ocean_shared class.
     *
     * @param south 		Minimum latitude to extract (deg).
     * @param north 		Maximum latitude to extract (deg).
     * @param west 			Minimum longitude to extract (deg).
     * @param east 			Maximum longitude to extract (deg).
     * @param month 		Month of the year for WOA extraction (1-12).
     * @param wind_speed    Wind_speed used to develop rough seas (m/s).
     * @param bottom_type 	Bottom type for rflect_loss_rayleigh model.
     */
    static void make_basic(
        double south, double north, double west, double east, int month,
        double wind_speed = 5,
        bottom_type_enum bottom_type = bottom_type_enum::sand);
};

/// @}
}  // end of namespace ocean
}  // end of namespace usml
