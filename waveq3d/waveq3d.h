/**
 * @file waveq3d.h WaveQ3D Model
 * @defgroup waveq3d WaveQ3D Model
 *
 * The WaveQ3D model computes propagation loss using hybrid Gaussian beams
 * in spherical/time coordinates. This model is designed to support real-time,
 * sonar simulation/stimulation systems, in littoral environments,
 * at active sonar frequencies.
 * \link usml::waveq3d::wave_queue
 * See wave_queue class for high level class diagram.
 * \endlink
 *
 * - Ray solutions to the eikonal equation are computed in latitude, longitude,
 *   and altitude coordinates to match wide-area environmental databases.
 *   This avoids the need to translate sound speed and bottom parameters into
 *   a series of 2-DxN radials.  Avoiding this step results in a significant
 *   computational savings, especially when the dynamics of the scenario
 *   precludes the reuse of environment parameters over multiple runs.
 *
 * - Hybrid Gaussian Beam techniques for propagation loss calculation are used
 *   to extend the applicability of ray theory to lower frequency regimes.
 *   The Gaussian Ray Bundling (GRAB) model has successfully used these
 *   techniques to support frequencies as low as 150 Hz.
 *
 * - Numerical integration of the eikonal equation is performed in the time
 *   dimension to maintain the phase continuity of the wavefront.  This choice
 *   speeds up the eigenray computation by allowing a single target collision
 *   to serve as the basis for multiple Gaussian beam calculations.
 *
 * - This 3-D approach also supports out-of-plane reflection from the ocean
 *   bottom.  Models that use 2-D environments over a series of radials cannot
 *   support this littoral effect.
 *
 * There are several limiting assumptions built into the current
 * implementation that may limit its utility for some applications.
 *
 * - The ocean data package defines the interfaces between this model and
 *   representations of the synthetic natural environment. This package also
 *   includes example implementations that have been derived from public
 *   sources. Implementations of limited distribution ocean models, such as
 *   those found in the U.S. Navy's Oceanographic and Atmospheric Master
 *   Library (OAML) are expected to be supported in a package that is external
 *   to the core model.
 *
 * - The gridding of launch angles and wavefront times can cause small
 *   scale features of the environment to be skipped over. In this way,
 *   the method of characteristics used by all ray theories is a spatial
 *   application of the Nyquist-Shannon sampling theorem that limits
 *   the accuracy of any digital representations of an analog processes.
 *   Developers should analyze the sensitivity of ray gridding choices
 *   to their own environmental scenarios.
 *
 * - Gaussian beam computation of propagation loss does not produce
 *   valid answers in the outermost cells of the D/E and AZ grid. This
 *   is because the model needs a supporting Gaussian beam on either side
 *   to maintain proper normalization. For this reason, all ray fans
 *   should include at least 3 rays in each launch angle dimension.
 *
 * - The model operates at its maximum efficiency when there are a large
 *   number of targets for each sensor, but the number of targets is
 *   much smaller than the number of environmental data points. For
 *   example, the model should produce valid answers for full field
 *   propagation loss estimates at all depths and ranges, but it may
 *   run significantly slower and with significantly greater memory
 *   requirements than a scenario with just ship and submarine targets.
 *
 * - The accuracy of the attenuation calculation is limited by the
 *   accuracy of the path length estimate between wavefronts. The
 *   current implementation uses straight line paths between equivalent
 *   points on the wavefront. This design decision was made to speed
 *   up the computation of low and mid-frequency Navy applications,
 *   where the effect of this approximation should be small. It's
 *   impact on weapon frequency propagation modeling has not yet
 *   been evaluated.
 *
 * - The UBLAS library requires gcc -03 optimization and the NDEBUG
 *   symbol defined to take full advantage of its computation speed
 *   features. The UBLAS library does not appear to produce
 *   correct answers at optimizations lower than the -O1 level.
 *
 * - Future efforts to use this implementation to model reverberation
 *   will integrate that feature into the interface scattering logic.
 *   Traditionally reverberation calculations are performed by re-computing
 *   propagation loss to every point on the interface or scattering layer.
 *   But doing so in this model violates the optimization assumption
 *   that the number of targets is small relative to the number of data
 *   points in the environment.
 *
 * - Target D/E has known small errors in the shadow zone when refraction is
 *   very strong.  The model currently uses the direction of the nearest part
 *   of the wavefront. But, it should be limited to the values from applying
 *   Snell's law to the critical angle cos(At)/c(Zt) = cos(As)/c(Zs).
 *   For example, in the pedersen_shallow_proploss test, the target D/E that
 *   decreases with range at 0.03 deg/yd instead of converging to a constant
 *   value. This limitation may need to be fixed in a future version.
 *
 * - The model doesn't include a GRAB-like algorithm to automatically increase
 *   ray spacing near caustics, surface duct axis, SOFAR axis.
 *   GRAB's SRC_ANG2 subroutine seems to assume that the current sound speed
 *   applies to all ranges for this part of the calculation. Without this
 *   correction, travel times for paths farther from the caustic
 *   can include a few milliseconds of error.  This error in travel time
 *   leads to significant fluctuation in the coherent propagation loss.
 *   This limitation may need to be fixed in a future version.
 *
 * These assumptions should be reviewed by the system development team
 * before this model is evaluated for their intended use.
 *
 * @defgroup waveq3d_test Regression Tests
 * @ingroup waveq3d
 *
 * Regression tests for the waveq3d package
 */
#ifndef USML_WAVEQ3D_H
#define USML_WAVEQ3D_H

#include <usml/waveq3d/wave_queue.h>
#include <usml/waveq3d/wave_front.h>
#include <usml/waveq3d/eigenray.h>
#include <usml/waveq3d/proploss.h>

#endif
