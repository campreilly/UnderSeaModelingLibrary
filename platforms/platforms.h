/**
 * @file platforms.h
 * Physical objects that move through the simulation.
 *
 * @defgroup platforms Platforms
 *
 * This package defines the creation and movement of platforms in the
 * simulation. The platform_model manages the motion of these objects and
 * manages the existence, position, and orientation of a list of child
 * platforms. Children's positions and orientations in world coordinates are
 * updated each time that the platform's position or orientation changes.
 *
 * Sensors are platforms that are can automatically compute there own acoustics.
 * Simple sensors can be implemented as objects that control their own motion,
 * or they can be attached to host platforms. Uses update_notifier to notify
 * listeners when eigenray and eigenverb data has changed.
 *
 * @defgroup platforms_test Regression Tests
 * @ingroup platforms
 *
 * Regression tests for the platforms package.
 */
#pragma once

#include <usml/platforms/motion_thresholds.h>
#include <usml/platforms/platform_manager.h>
#include <usml/platforms/platform_model.h>
#include <usml/platforms/sensor_model.h>
#include <usml/wavegen/wavefront_generator.h>
#include <usml/wavegen/wavefront_listener.h>
