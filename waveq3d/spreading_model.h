/**
 * @file spreading_model.h
 * Spreading loss component of propagation loss.
 */
#pragma once

#include <usml/types/wposition1.h>
#include <usml/usml_config.h>
#include <usml/waveq3d/wave_queue.h>

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <cstddef>

namespace usml {
namespace waveq3d {

using namespace usml::ocean;

/**
 * @internal
 * A spreading loss model computes the spreading component of
 * propagation loss. These models compute their results as a
 * function of frequency to support broadband acoustics.
 */
class USML_DECLSPEC spreading_model {
    friend class wave_queue;

   protected:
    /** Wavefront object associated with this model. */
    wave_queue& _wave;

    /** Frequency dependent part of beam spreading. */
    vector<double> _spread;

    /**
     * Initial ensonified area for each ray span. Assign the area for each
     * span to the index of the ray that precedes it in D/E and azimuth.
     * Copy the last element in each direction from  the one before it.
     */
    matrix<double> _init_area;

    /**
     * Initializes the spreading model.
     *
     * @param wave          Wavefront object associated with this model.
     * @param num_freqs     Number of different frequencies.
     */
    spreading_model(wave_queue& wave, size_t num_freqs)
        : _wave(wave),
          _spread(num_freqs),
          _init_area(wave.num_de(), wave.num_az()) {}

    /**
     * Virtual destructor
     */
    virtual ~spreading_model() {}

    /**
     * Estimate intensity at a specific target location.
     *
     * @param  location     Target location.
     * @param  de           DE index of closest point of approach.
     * @param  az           AZ index of closest point of approach.
     * @param  offset       Offsets in time, DE, and AZ at collision.
     * @param  distance     Offsets in distance units.
     * @return              Intensity of ray at this point.
     */
    virtual const vector<double>& intensity(const wposition1& location,
                                            size_t de, size_t az,
                                            const vector<double>& offset,
                                            const vector<double>& distance) = 0;

    /**
     * Interpolate the half-width of a cell in the D/E direction.
     *
     * @param   de          DE index of contributing cell.
     * @param   az          AZ index of contributing cell.
     * @param   offset      Offsets in time, DE, and AZ at collision.
     * @return              Half-width of cell in the DE direction.
     */
    virtual double width_de(size_t de, size_t az,
                            const vector<double>& offset) = 0;

    /**
     * Interpolate the half-width of a cell in the AZ direction.
     *
     * @param   de          DE index of contributing cell.
     * @param   az          AZ index of contributing cell.
     * @param   offset      Offsets in time, DE, and AZ at collision.
     * @return              Half-width of cell in the AZ direction.
     */
    virtual double width_az(size_t de, size_t az,
                            const vector<double>& offset) = 0;

    /**
     * Initial ensonified area for each ray span. Assign the area for each
     * span to the index of the ray that precedes it in D/E and azimuth.
     *
     * @param   de          DE index of contributing cell.
     * @param   az          AZ index of contributing cell.
     */
    double init_area(size_t de, size_t az) { return _init_area(de, az); }
};

}  // end of namespace waveq3d
}  // end of namespace usml
