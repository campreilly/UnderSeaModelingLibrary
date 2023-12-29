/**
 * @file transmit_model.h
 * Parameters for a single waveform in a transmission schedule.
 */
#pragma once

#include <usml/transmit/window.h>
#include <usml/types/bvector.h>
#include <usml/usml_config.h>

#include <boost/numeric/ublas/vector.hpp>
#include <complex>
#include <memory>
#include <string>
#include <list>

namespace usml {
namespace transmit {

using namespace usml::types;

/// @ingroup transmit
/// @{

/// Alias for vector of complex doubles.
typedef boost::numeric::ublas::vector<std::complex<double> > cdvector;

/**
 * Parameters for a single waveform in a transmission schedule. Subclasses
 * implement the asignal() method that builds the time series for each waveform
 * type. Attributes in this class are immutable to avoid any requirement to lock
 * access for multi-threaded applications.
 */
class USML_DECLSPEC transmit_model {
   public:
    /// Alias for shared const reference to transmit_model.
    typedef std::shared_ptr<const transmit_model> csptr;

    /// Display name for waveform type.
    const std::string type;

    /// Duration of the transmitted signal (sec).
    const double duration;

    /// Center frequency for current modulation scheme (Hz).
    const double fcenter;

    /// Frequency bandwidth for current modulation scheme (Hz).
    const double bandwidth;

    /// Delay after completion of previous pulse (sec).
    const double delay;

    /// Peak intensity of the transmitted pulse (micro Pa at 1m).
    const double source_level;

    /// Waveform shading window.
    const window::type window_type;

    /// Waveform shading window parameter.
    const double window_param;

    /// Ordered steering azimuth relative to host orientation (deg)
    const double orderedAZ;

    /// Ordered steering elevation relative to host orientation (deg)
    const double orderedDE;

    /// Transmitter beam pattern number to use
    const int transmit_mode;

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
    virtual cdvector asignal(double fsample, double fband, double inphase,
                             double* outphase) = 0;

   protected:
    /**
     * Build transmit_model from its constituent parts.
     *
     * @param type          Display name for waveform type.
     * @param duration      Duration of the transmitted signal (sec).
     * @param fcenter       Center frequency for current modulation scheme (Hz).
     * @param bandwidth     Frequency bandwidth for current modulation (Hz).
     * @param delay         Delay after completion of previous pulse (sec).
     * @param source_level  Peak intensity of pulse (microPa at 1m).
     * @param window_type   Waveform shading window.
     * @param window_param  Waveform shading parameter.
     * @param orderedAZ  	Ordered steering azimuth relative to host (deg).
     * @param orderedDE  	Ordered steering elevation relative to host (deg).
     * @param transmit_mode Transmitter beam pattern number to use.
     */
    transmit_model(const std::string& type, double duration, double fcenter,
                   double bandwidth, double delay, double source_level,
                   window::type window_type, double window_param,
                   double orderedAZ, double orderedDE, int transmit_mode);

    /**
     * Add window to transmit pulse, in-place.
     */
    void add_window(cdvector& signal) const;
};

/**
 * List of transmit pulses.
 */
typedef std::list<transmit_model::csptr> transmit_list;

/// @}
}  // namespace transmit
}  // namespace usml
