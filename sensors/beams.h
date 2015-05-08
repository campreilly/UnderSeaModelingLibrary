/**
 * @file beams.h Beam Pattern Models
 * @defgroup sensors Sensor Components
 *
 * This package defines the beam pattern models provided by USML.
 *
 * @defgroup beams Beam Pattern Models
 * @ingroup sensors
 *
 * A "beam_pattern_model" computes the pressure response of an
 * incident wave as a function of frequency and incident wave angle.
 * The Models can produce a beam level and directivity index of the
 * array they are modeling.
 *
 * @defgroup beams_test Regression Tests
 * @ingroup beams
 *
 * Regression tests for the beams package
 */
#pragma once

#include <usml/sensors/beam_pattern_model.h>

#include <usml/sensors/beam_pattern_omni.h>
#include <usml/sensors/beam_pattern_cosine.h>
#include <usml/sensors/beam_pattern_sine.h>

#include <usml/sensors/beam_pattern_line.h>
#include <usml/sensors/beam_pattern_HLA.h>
#include <usml/sensors/beam_pattern_VLA.h>

#include <usml/sensors/beam_pattern_solid.h>
#include <usml/sensors/beam_pattern_multi.h>
#include <usml/sensors/beam_pattern_grid.h>
