/**
 * @file biverbs.h Bistatic Eigenverbs
 * Combination of source and receiver eigenverbs for a bistatic pair.
 *
 * @defgroup biverbs biverbs
 *
 * Combination of source and receiver eigenverbs for a bistatic pair.
 * Pre-computes all of the geometry related elements of eigenverb overlap except
 * the application of the beam patterns. Assumes that the beam patterns many
 * change more quickly than the geometry of the source and receiver to each
 * scattering patch.
 *
 * @defgroup biverbs_test Bistatic Eigenverb Tests
 * @ingroup biverbs
 *
 * Regression tests for the biverbs module.
 */
#pragma once

#include <usml/biverbs/biverb_model.h>
#include <usml/biverbs/biverb_collection.h>
#include <usml/biverbs/biverb_generator.h>
