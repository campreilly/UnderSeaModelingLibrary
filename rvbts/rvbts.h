/**
 * @file rvbts.h Reverberation time series for a bistatic pair.
 * @defgroup rvbts rvbts
 *
 * This implementation supports beam level simulations where each receiver
 * channel has its own beam pattern and steering. It lacks the phase delay
 * between channels needed to support element level simulation.
 *
 * @defgroup rvbts_test rvbts Tests
 * @ingroup rvbts
 *
 * Regression tests for the rvbts module.
 */
#pragma once

#include <usml/rvbts/rvbts_collection.h>
#include <usml/rvbts/rvbts_generator.h>
