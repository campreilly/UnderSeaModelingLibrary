/**
 * @file motion_thresholds.h
 * Thresholds that causes new data for acoustics to be generated.
 */

#include <usml/platforms/motion_thresholds.h>

using namespace usml::platforms;

const double motion_thresholds::lat_threshold = 0.01;   // degrees
const double motion_thresholds::lon_threshold = 0.01;   // degrees
const double motion_thresholds::alt_threshold = 5.0;    // meters
const double motion_thresholds::yaw_threshold = 5.0;    // degrees
const double motion_thresholds::pitch_threshold = 5.0;  // degrees
const double motion_thresholds::roll_threshold = 5.0;   // degrees
