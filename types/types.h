/**
 * @file types.h Fundamental Data Types
 * @defgroup types Fundamental Data Types
 *
 * This package defines the basic data types specific to USML.
 *
 * @defgroup wposition World Coordinates
 * @ingroup types
 *
 * Locations and directions are defined in a spherical earth
 * coordinate system relative to the WGS84 center of curvature for the
 * local area of operations. The model also supports the ability to
 * transform these coordinates into the geodetic parameters of
 * latitude, longitude, and altitude.
 *
 * @defgroup data_grid Data Grids and Sequences
 * @ingroup types
 *
 * These components define support for N-dimensional data sets and their
 * associated axes. Supports interpolation in any number of dimensions.
 * Fast interpolation algorithms require an ability to quickly lookup an
 * axis index appropriate given a floating point axis value.
 * These axes are implemented as read-only, monotonic sequence of values.
 *
 * @defgroup min_grid Numerical Minimization
 * @ingroup types
 *
 * These components define support for N-dimensional algorithms for
 * numerical minimization.  The min_grid has a seq_vector
 * axes in each dimension, and the field is defined at each point in this grid.
 * The gridded values are used to search for the axis offsets that would
 * result in a local minimum in the interpolated field.  This algorithm assumes
 * that only a single local minimum for the interpolated field exists in
 * the neighborhood around the minimum in the gridded field.
 *
 * @defgroup types_test Regression Tests
 * @ingroup types
 *
 * Regression tests for the types package
 */
#pragma once

#include <usml/types/wvector.h>
#include <usml/types/wvector1.h>
#include <usml/types/wposition.h>
#include <usml/types/wposition1.h>

#include <usml/types/seq_linear.h>
#include <usml/types/seq_log.h>
#include <usml/types/seq_data.h>
#include <usml/types/seq_rayfan.h>

#include <usml/types/data_grid.h>
#include <usml/types/data_grid_bathy.h>
#include <usml/types/data_grid_svp.h>
