/**
 * @file bp_model.cc
 * Base class used for building beam pattern models.
 */

#include <usml/beampatterns/bp_model.h>

#include <cmath>
#include <utility>

using namespace usml::beampatterns;

/**
 * Computes the beam level gain for an arrival vector in the body coordinates
 * of an array which has been rotated by 'orient'.
 */
void bp_model::beam_level(const bvector& arrival, const orientation& orient,
                          const seq_vector::csptr& frequencies, vector<double>* level,
                          const bvector& steering, double sound_speed) const {
    bvector rotated;
    rotated.rotate(orient, arrival);
    this->beam_level(rotated, frequencies, level, steering, sound_speed);
}

/**
 * Computes the directivity gain for this beam pattern.
 */
void bp_model::directivity(const seq_vector::csptr& frequencies, vector<double>* level,
                           const bvector& steering, double sound_speed) const {
    // loop over all solid angles

    vector<double> beam(frequencies->size(), 0.0);
    const double dangle = M_PI / 180.0;  // both dtheta and dphi
    for (double az = 0.0; az <= TWO_PI; az += dangle) {
        for (double de = -M_PI_2; de <= M_PI_2; de += dangle) {
            // compute angle terms
            const double cos_de = cos(de);
            const double sin_de = sin(de);
            const double cos_az = cos(az);
            const double sin_az = sin(az);

            // compute beam level at this DE and AZ angle
            bvector arrival(cos_de * cos_az, cos_de * sin_az, sin_de);
            beam_level(arrival, frequencies, &beam, steering, sound_speed);

            // add contribution to integral at each frequency
            *level += beam * (cos_de * dangle * dangle);
        }
    }

    // normalize level to unit sphere
    *level = (*level) / (4 * M_PI);
}
