/**
 * @file motion_thresholds.h
 * Thresholds that causes new data for acoustics to be generated.
 */
#pragma once

namespace usml {
namespace platforms {

/// @ingroup platforms
/// @{

/**
 * Thresholds that causes new data for acoustics to be generated.
 */
class motion_thresholds {
   public:
    /** Maximum change in latitude (degrees). */
    static const double lat_threshold;

    /** Maximum change in longitude  (degrees). */
    static const double lon_threshold;

    /** Maximum change in altitude  (meters). */
    static const double alt_threshold;

    /** Maximum change in yaw  (degrees). */
    static const double yaw_threshold;

    /** Maximum change in pitch  (degrees). */
    static const double pitch_threshold;

    /** Maximum change in roll  (degrees). */
    static const double roll_threshold;
};

/// @}
}  // namespace platforms
}  // namespace usml
