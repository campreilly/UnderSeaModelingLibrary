/**
 * @file bp_arb.h
 * Models a beam pattern based on arbitrary 3D element locations.
 */
#pragma once

#include <usml/beampatterns/bp_model.h>
#include <usml/types/bvector.h>
#include <usml/types/seq_vector.h>
#include <usml/usml_config.h>

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <complex>

namespace usml {
namespace beampatterns {

/// @ingroup beampatterns
/// @{

/**
 * Models a beam pattern based on arbitrary 3D element locations and weights.
 * The general equation for the beam pattern of N elements with arbitrary
 * 3D locations and complex weights is the normalized power of a
 * Discreet Fourier Transform (DFT) in wavenumber space:
 * \f[
 *      B(\vec{u},\vec{u}_s,\lambda) =
 *      \frac{ \left| \sum_{n=1}^N w_n \exp \left[
 *          i \frac{2 \pi}{\lambda} \Delta \vec{u} \cdot \vec{r}_n
 *          \right] \right|^2 }
 *      { \left| \sum_{n=1}^N w_n \right|^2 }
 * \f]
 * where
 * - \f$ \vec{r}_n \f$ = 3D location of each array element,
 * - \f$ \vec{u} \f$   = wavefront direction (out from array),
 * - \f$ \vec{u}_s \f$ = array steering direction,
 * - \f$ \Delta\vec{u} = \vec{u} - \vec{u}_s \f$,
 * - \f$ \lambda \f$   = wavelength for incoming plane wave,
 * - \f$ w_n \f$       = complex weight for each array element, and
 * - \f$ N \f$         = total number of array elements.
 *
 * This implementation computes the DFT summation for every call to
 * beam_level(). It is perfectly accurate, but can be slow if the number of
 * elements is large. The bp_k_grid class models arbitrary arrays faster, but it
 * is less accurate.
 */
class USML_DECLSPEC bp_arb : public bp_model {
   public:
    /**
     * Constructs a beam pattern based on arbitrary 3D element locations
     * with complex weights and a uniform element pattern.
     *
     * @param elem_locs     An Nx3 matrix of element locations in front, right,
     *                      up coordinates.  The matrix will be copied during
     *                      instantiation.
     * @param weights       A N length vector of weight to be applied per
     *                      element, defaults to uniform.
     * @param back_baffle   Set gain to zero in backplane when true.
     */
    bp_arb(const matrix<double> &elem_locs,
           const vector<std::complex<double> > &weights,
           bool back_baffle = false)
        : _N_elements(elem_locs.size1()),
          _elem_locs(elem_locs),
          _weights(weights),
          _back_baffle(back_baffle) {}

    /**
     * Constructs a beam pattern based on arbitrary 3D element locations
     * with complex weights and a uniform element pattern.
     *
     * @param elem_locs     An Nx3 matrix of element locations in front, right,
     *                      up coordinates.  The matrix will be copied during
     *                      instantiation.
     * @param weights       A N length vector of weight to be applied per
     *                      element, defaults to uniform.
     * @param back_baffle   Set gain to zero in backplane when true.
     */
    bp_arb(const matrix<double> &elem_locs, const vector<double> &weights,
           bool back_baffle = false)
        : _N_elements(elem_locs.size1()),
          _elem_locs(elem_locs),
          _weights(weights),
          _back_baffle(back_baffle) {}

    /**
     * Constructs a beam pattern based on arbitrary 3D element locations
     * with uniform weights and a uniform element pattern.
     *
     * @param elem_locs     An Nx3 matrix of element locations in front, right,
     *                      up coordinates.  The matrix will be copied during
     *                      instantiation.
     * @param back_baffle   Set gain to zero in backplane when true.
     */
    bp_arb(const matrix<double> &elem_locs, bool back_baffle = false)
        : _N_elements(elem_locs.size1()),
          _elem_locs(elem_locs),
          _weights(vector<double>(elem_locs.size1(), 1.0)),
          _back_baffle(back_baffle) {}

    void beam_level(const bvector &arrival,
                    const seq_vector::csptr &frequencies, vector<double> *level,
                    const bvector &steering = bvector(1.0, 0.0, 0.0),
                    double sound_speed = 1500.0) const override;

   private:
    /// The number elements in the array.
    const double _N_elements;

    /// An _N_elements x 3 matrix of element locations in meters.
    const matrix<double> _elem_locs;

    /// An _N_elements vector of weights to be applied to each element.
    const vector<std::complex<double> > _weights;

    /// Set gain to zero in backplane when true.
    const bool _back_baffle;
};

/// @}
}  // namespace beampatterns
}  // namespace usml
