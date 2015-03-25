/**
 * @file sensors.h 
 * @defgroup sensors Fundamental Data sensors
 *
 * This package defines the basic data sensors specific to USML.
 *
 * @defgroup wposition World Coordinates
 * @ingroup sensors
 *
 * Locations and directions are defined in a spherical earth
 * coordinate system relative to the WGS84 center of curvature for the
 * local area of operations. The model also supports the ability to
 * transform these coordinates into the geodetic parameters of
 * latitude, longitude, and altitude.
 *
 * @defgroup data_grid Data Grids and Sequences
 * @ingroup sensors
 *
 * These components define support for N-dimensional data sets and their
 * associated axes. Supports interpolation in any number of dimensions.
 * Fast interpolation algorithms require an ability to quickly lookup an
 * axis index appropriate given a floating point axis value.
 * These axes are implemented as read-only, monotonic sequence of values.
 *
 * @defgroup sensors_test Regression Tests
 * @ingroup sensors
 *
 * Regression tests for the sensors package
 */
#pragma once

#include <usml/sensors/beam_pattern_model.h>
#include <usml/sensors/beam_pattern_map.h>
#include <usml/sensors/beam_pattern_model.h>

#include <usml/sensors/paramsIDType.h>
#include <usml/sensors/receiver_params.h>
#include <usml/sensors/receiver_params_map.h>
#include <usml/sensors/source_params.h>
#include <usml/sensors/source_params_map.h>

#include <usml/sensors/xmitRcvModeType.h>
#include <usml/sensors/sensorIDType.h>
#include <usml/sensors/sensor.h>
#include <usml/sensors/sensor_map.h>
#include <usml/sensors/sonobuoy.h>
