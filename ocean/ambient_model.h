/**
 * @file ambient_model.h
 * Generic interface for ambient noise in the ocean.
 */
#pragma once

#include <usml/types/seq_vector.h>
#include <usml/types/wposition1.h>
#include <usml/usml_config.h>

#include <boost/numeric/ublas/vector.hpp>
#include <memory>

namespace usml {
namespace ocean {

using namespace usml::types;

/// @ingroup profiles
/// @{

/**
 * The ambient_model class, and its set of sub classes, are used for calculating
 * ambient noise in the ocean. Each sub-class implements the 'ambient' function
 * to calculate power spectral density of ambient noise. For most models
 * the 0Hz bin is ignored. If you request information from this bin the
 * function will return -inf dB.
 */
class USML_DECLSPEC ambient_model {
   public:
    /// Shared pointer to constant version of this class.
    typedef std::shared_ptr<const ambient_model> csptr;

    /**
     * Computes the power spectral density of ambient noise at a specific
     * location.
     *
     * @param location  Location at which to compute noise.
     * @param frequency Frequencies at which noise is calculated.
     * @param noise	    Ambient noise power spectral density (output)
     */
    virtual void ambient(const wposition1 &location,
                         seq_vector::csptr frequency,
                         vector<double> *noise) const = 0;

    /**
     * Virtual destructor
     */
    virtual ~ambient_model() {}
};

/// @}
}  // end of namespace ocean
}  // end of namespace usml
