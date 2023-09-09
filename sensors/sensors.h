/**
 * @file bistatic.h
 * Modeling products for links between sources and receivers.
 * @defgroup bistatic Bistatic Sensor Pairs
 *
 * Sensors are platforms that are can automatically compute there own acoustics.
 * Simple sensors can be implemented as objects that control their own motion,
 * or they can be attached to host platforms. Uses update_notifier to notify
 * listeners when eigenray and eigenverb data has changed.
 *
 * This package defines the objects that store bistatic eigenray and eigenverb
 * data products. Each eigenray represents a single acoustic path between a
 * source and target. The direct paths are eigenrays that connect this source
 * and receiver; they represent the multipath direct blast contributions to the
 * received signal. Eigenverbs are a Gaussian beam projection of an acoustic ray
 * onto a reverberation interface at the point of collision. The bistatic
 * eigenverbs (biverbs) represent the bistatic overlap between the source and
 * receiver eigenverbs for this pair.
 *
 * @defgroup bistatic_test Regression Tests
 * @ingroup bistatic
 *
 * Regression tests for the bistatic module.
 *
 */
#pragma once

#include <usml/sensors/sensor_manager.h>
#include <usml/sensors/sensor_pair.h>
