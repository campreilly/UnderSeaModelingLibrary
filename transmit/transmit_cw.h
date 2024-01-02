/**
 * @file transmit_cw.h
 * Pulsed continuous wave (CW) transmission.
 */
#pragma once

#include <usml/transmit/transmit_model.h>
#include <usml/transmit/window.h>
#include <usml/usml_config.h>

#include <string>

namespace usml {
namespace transmit {

/// @ingroup transmit
/// @{

/**
 * Waveform parameters for a pulsed continuous wave pulse. It implements the
 * equation:
 * \f[
 *      s[n] = exp\left[ i ( 2 \pi (f_c-f_b) n / f_s + \phi_0 ) \right]
 * \f]
 * where:
 * - \f$ f_c \f$ is the center frequency of the pulse,
 * - \f$ f_b \f$ is the center of the frequency band for complex basebanding,
 * - \f$ f_s \f$ is the sampling rate for digital waveform,
 * - \f$ \phi_0 \f$ is the initial phase of the pulse,
 * - \f$ n \f$ is the index number of the signal output.
 */
class USML_DECLSPEC transmit_cw : public transmit_model {
   public:
    /**
     * Build waveform from its constituent parts.
     *
     * @param type          Display name for waveform type.
     * @param duration      Duration of the transmitted signal (sec).
     * @param fcenter       Center frequency for current modulation scheme (Hz).
     * @param delay         Delay after completion of previous pulse (sec).
     * @param source_level  Peak intensity of the transmitted pulse (microPa at
     * 1m).
     * @param window        Waveform shading window.
     * @param window_param  Waveform shading parameter.
     * @param orderedAZ  	Ordered steering azimuth direction (deg).
     * @param orderedDE  	Ordered steering elevation direction (deg)
     * @param transmit_mode Transmitter beam pattern number to use.
     */
    transmit_cw(const std::string& type, double duration, double fcenter,
                double delay = 0.0, double source_level = 1.0,
                window::type window = window::type::BOXCAR,
                double window_param = 0.0, double orderedAZ = 0.0,
                double orderedDE = 0.0, int transmit_mode = 0);

    /**
     * Creates a complex analytic signal for this waveform. To support phase
     * continuity between waveforms, this implement includes an option to
     * take the end phase of each waveform and pass it to the next pulse.
     *
     * @param fsample   Sampling rate for digital waveform (Hz).
     * @param fband     Center of the frequency band for complex basebanding.
     * @param inphase   Phase at which to start the signal.
     * @param outphase  Phase at which the next signal should start.
     */
    cdvector asignal(double fsample, double fband, double inphase = 0.0,
                     double* outphase = nullptr) override;
};

/// @}
}  // namespace transmit
}  // namespace usml
