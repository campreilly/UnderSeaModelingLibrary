/**
 * @file ocean.h Ocean Components
 * @defgroup ocean Ocean Components
 *
 * This package defines the interface between WaveQ3D and the
 * models used to describe the synthetic natural environment.
 * This package also includes example implementations that have
 * been derived from public sources.  Implementations of
 * limited distribution ocean models, such as those found in the
 * U.S. Navy's Oceanographic and Atmospheric Master Library (OAML)
 * should be specified in a separate package.
 *
 * @defgroup profiles Ocean Profiles
 * @ingroup ocean
 *
 * A "profile model" computes the environmental parameters of
 * ocean water. The modeled properties include the sound velocity
 * profile and the attenuation due to sea water absorption.
 * This class implements an attenuation model through delegation.
 * The delegated model is defined separately and added to its host
 * during/after construction.  The host is defined as an attenuation_model
 * subclass so that its children can share the attenuation model
 * through this delegation.
 *
 * @defgroup boundaries Ocean Boundaries
 * @ingroup ocean
 *
 * A "boundary model" computes the environmental parameters of
 * the ocean's surface or bottom.  The modeled properties include
 * the depth and reflection properties of the interface.
 * This class implements a reflection loss model through delegation.
 * The delegated model is defined separately and added to its host
 * during/after construction.  The host is defined as an reflect_loss_model
 * subclass so that its children can share the reflection loss model
 * through this delegation.
 *
 * @defgroup ocean_model Ocean Model
 * @ingroup ocean
 *
 * Combines the effects of surface, bottom, volume, and profile into a single model.
 *
 *      - ocean_model.h
 *
 * @defgroup ocean_test Regression Tests
 * @ingroup ocean
 *
 * Regression tests for the ocean package
 */
#pragma once

#include <usml/ocean/attenuation_model.h>
#include <usml/ocean/attenuation_constant.h>
#include <usml/ocean/attenuation_thorp.h>

#include <usml/ocean/profile_model.h>
#include <usml/ocean/profile_linear.h>
#include <usml/ocean/profile_munk.h>
#include <usml/ocean/profile_n2.h>
#include <usml/ocean/profile_catenary.h>
#include <usml/ocean/profile_grid.h>
#include <usml/ocean/profile_grid_fast.h>
#include <usml/ocean/ascii_profile.h>
#include <usml/ocean/data_grid_mackenzie.h>

#include <usml/ocean/reflect_loss_model.h>
#include <usml/ocean/reflect_loss_constant.h>
#include <usml/ocean/reflect_loss_netcdf.h>
#include <usml/ocean/reflect_loss_rayleigh.h>
#include <usml/ocean/reflect_loss_rayleigh_grid.h>
#include <usml/ocean/reflect_loss_eckart.h>
#include <usml/ocean/reflect_loss_beckmann.h>

#include <usml/ocean/boundary_model.h>
#include <usml/ocean/boundary_flat.h>
#include <usml/ocean/boundary_slope.h>
#include <usml/ocean/boundary_grid.h>
#include <usml/ocean/boundary_grid_fast.h>
#include <usml/ocean/ascii_arc_bathy.h>
#include <usml/ocean/wave_height_pierson.h>

#include <usml/ocean/scattering_model.h>
#include <usml/ocean/scattering_constant.h>
#include <usml/ocean/scattering_lambert.h>

#include <usml/ocean/volume_model.h>
#include <usml/ocean/volume_flat.h>

#include <usml/ocean/ocean_model.h>
