/**
 * @file bp_line.h
 * Vertical and horizontal line arrays in closed form.
 */
#pragma once

#include <usml/beampatterns/bp_model.h>
#include <usml/types/bvector.h>
#include <usml/types/seq_vector.h>
#include <usml/usml_config.h>

#include <boost/numeric/ublas/vector.hpp>

namespace usml {
namespace beampatterns {

/// @ingroup beampatterns
/// @{

/// Line array type.
enum bp_line_type {
    HLA = 0,  ///< horizontal line array
    VLA = 1   ///< vertical line array
};

/**
 * Implements the closed for solution of a line array.
 *
 * Horizontal arrays are along the front axis and vertical arrays are along the
 * up axis.
 *
 * \f[
 *      b_{hla}(n,e,u,f,\phi_s) =
 *      \left |
 *      \frac{   sin( N \frac{\pi d}{\lambda}  (n - cos(\phi_s)) ) }
 *           { N sin(   \frac{\pi d}{\lambda}  (n - cos(\phi_s)) ) }
 *      \right |
 * \f]
 *
 * \f[
 *      b_{vla}(n,e,u,f,\theta_s) =
 *      \left |
 *      \frac{   sin( N \frac{\pi d}{\lambda}  (u - sin(\theta_s)) ) }
 *           { N sin(   \frac{\pi d}{\lambda}  (u - sin(\theta_s)) ) }
 *      \right |
 * \f]
 *
 * Where n,e,u and the front, right, and up compoenents of an arrival vector.
 *
 * The directivity gain for planar array can be estimated using:
 * \f[
 * DG = \frac{1}{N^2} \sum_{n=1}^N \sum_{m=1}^N
 * \left[ \delta_{nm} + (1-\delta_{nm}) \cos( \alpha \pi (n-m) cos(\phi_s))
 * \frac{ \sin( \alpha \pi (n-m) )}{ \alpha \pi (n-m) } \right]
 * \f]
 * where N is the number of elements, \f$\alpha\f$ is the ratio of
 * spacing relative to half wavelength \f$(\alpha=d/(\lambda/2))\f$. and
 * \f$\phi_s\f$ is the steering angle relative to first element.
 * When the beam pattern is computed at its design frequency
 * (where \f$\alpha=1\f$), the DI is inversely proportional to the
 * number of elements at all steerings.  But, the full equation includes
 * frequency dependent effects, including increases in DI near end-fire.
 *
 * @xref Albert H. Nuttall, Benjamin A. Cray, Approximations to
 * Directivity for Linear, Planar, and Volumetric Apertures and Arrays,
 * NUWC-NPT Technical Report 10,798, 25 July 1997, Equation 17.
 */
class USML_DECLSPEC bp_line : public bp_model {
   public:
    /**
     * Initialize array parameters.
     *
     * @param num_elements  number of receiver in array
     * @param spacing       element spacing in meters
     * @param type          HLA or VLA
     */
    bp_line(unsigned num_elements, double spacing, bp_line_type type)
        : _num_elements(num_elements), _spacing(spacing), _type(type) {}

    virtual void beam_level(const bvector& arrival,
                            const seq_vector::csptr& frequencies,
                            vector<double>* level,
                            const bvector& steering = bvector(1.0, 0.0, 0.0),
                            double sound_speed = 1500.0) const;

    virtual void directivity(const seq_vector::csptr& frequencies,
                             vector<double>* level,
                             const bvector& steering = bvector(1.0, 0.0, 0.0),
                             double sound_speed = 1500.0) const;

   private:
    /** Number of elements in the line array. */
    const double _num_elements;

    /** Spacing between elements in the line array. */
    const double _spacing;

    /** VLA or HLA */
    const bp_line_type _type;
};

/// @}
}  // end of namespace beampatterns
}  // end of namespace usml
