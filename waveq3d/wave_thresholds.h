/**
 * @file wave_thresholds.h
 * Computational thresholds for the WaveQ3D model.
 */
#pragma once

#include <usml/waveq3d/wave_front.h>
#include <boost/foreach.hpp>

namespace usml {
namespace waveq3d {

/// @ingroup waveq3d
/// @{

/**
 * Computational thresholds for the WaveQ3D model.  These thresholds allow
 * the simulation application to define the minimum intensity levels
 * for valid eigenrays and eigenverbs.  They also allow the simulation
 * application to control the maximum number of bounces in valid eigenrays
 * and eigenverbs.
 *
 * This implementation is completely defined in the header file so that
 * all methods can be defined inline.
 */
class USML_DECLSPEC wave_thresholds {

  public:

	/**
	 * Set thresholds to default values that a designed to let almost
	 * everything through. The intensity_threshold and eigenverb_threshold
	 * default to -300 dB. The maximum number of bottom, surface, caustic,
	 * upper, and lower default to 999.
	 */
	wave_thresholds() :
		_intensity_threshold(300.0),
		_eigenverb_threshold(1e-30),
		_max_bottom(999),
		_max_surface(999),
		_max_caustic(999),
		_max_upper(999),
		_max_lower(999)
	{
	}

    /**
     * The value of the eigenray intensity threshold in dB.
     * Any eigenray with an intensity value that is weaker
     * than this threshold is not sent the eigenray listeners.
     * Stored as a positive value in dB for later comparison
	 * with the positive eigenray.intensity value.
     */
	inline void intensity_threshold(double max) {
		_intensity_threshold = abs(max);
	}

    /**
     * The value of the eigenray intensity threshold in dB.
     * Any eigenray with an intensity value that is weaker
     * than this threshold is not sent the eigenray listeners.
     */
	inline double intensity_threshold() {
		return -_intensity_threshold;
	}

    /**
     * Test a list of eigenray.intensity values against the intensity threshold.
     *
     * @param  intensities	List of intensities for use by BOOST_FOREACH().
     * 						Assumed to be positive loss values in dB units.
     * @return false is there are no values above this threshold.
     */
	template<class T> bool above_intensity_threshold( T intensities ) {
		BOOST_FOREACH( double level, intensities ) {
			if ( level < _intensity_threshold ) return true ;
		}
		return false ;
	}

    /**
     * The value of the eigenverb power threshold in dB.
     * Any eigenray with an power value that is weaker
     * than this threshold is not sent the eigenray listeners.
     * Stored in linear units as a value between zero and one
     * for later comparison with the verb.power value.
     */
	inline void eigenverb_threshold(double max) {
		_eigenverb_threshold = pow(10.0,-0.1*abs(max)) ;
	}

    /**
     * The value of the eigenverb power threshold in dB.
     * Any eigenray with a power value that is weaker
     * than this threshold is not sent the eigenray listeners.
     */
	inline double eigenverb_threshold() {
		return 10.0 * log10(_eigenverb_threshold) ;
	}

    /**
     * Test a list of eigenverb.power values against the eigenverb threshold.
     * Both values are assumed to be in linear units as a value between zero and one.
     *
     * @param  power	List of powers for use by BOOST_FOREACH().
     * @return false is there are no values above this threshold.
     */
	template<class T> bool above_eigenverb_threshold( T power ) {
		BOOST_FOREACH( double level, power ) {
			if ( level >= _eigenverb_threshold ) return true ;
		}
		return false ;
	}

    /**
     * The maximum number of bottom bounces.
     * Any eigenray or eigenverb with more than this number
     * of bottom bounces is not sent the listeners.
     * Defaults to 999.
     */
    inline int max_bottom() const {
    	return _max_bottom ;
    }

    /**
     * The maximum number of bottom bounces.
     * Any eigenray or eigenverb with more than this number
     * of bottom bounces is not sent the listeners.
     */
    inline void max_bottom( int max ) {
    	_max_bottom = max ;
    }

    /**
     * The maximum number of surface bounces.
     * Any eigenray or eigenverb with more than this number
     * of surface bounces is not sent the listeners.
     * Defaults to 999.
     */
    inline int max_surface() const {
    	return _max_surface ;
    }

    /**
     * The maximum number of surface bounces.
     * Any eigenray or eigenverb with more than this number
     * of surface bounces is not sent the listeners.
     */
    inline void max_surface( int max ) {
    	_max_surface = max ;
    }

    /**
     * The maximum number of caustic turning points.
     * Any eigenray or eigenverb with more than this number
     * of caustic turning points is not sent the listeners.
     * Defaults to 999.
     */
    inline int max_caustic() const {
    	return _max_caustic ;
    }

    /**
     * The maximum number of caustic turning points.
     * Any eigenray or eigenverb with more than this number
     * of caustic turning points is not sent the listeners.
     */
    inline void max_caustic( int max ) {
    	_max_caustic = max ;
    }

    /**
     * The maximum number of upper vertices.
     * Any eigenray or eigenverb with more than this number
     * of caustic turning points is not sent the listeners.
     * Defaults to 999.
     */
    inline int max_upper() const {
    	return _max_upper ;
    }

    /**
     * The maximum number of upper vertices.
     * Any eigenray or eigenverb with more than this number
     * of upper vertices is not sent the listeners.
     */
    inline void max_upper( int max ) {
    	_max_upper = max ;
    }

    /**
     * The maximum number of lower vertices.
     * Any eigenray or eigenverb with more than this number
     * of lower vertices is not sent the listeners.
     * Defaults to 999.
     */
    inline int max_lower() const {
    	return _max_lower ;
    }

    /**
     * Test the number of bounces in wave_front ray path against thresholds.
     *
     * @param  wave		The wave_front ray path to be tested.
     * @param  de		Index number of D/E angle to test.
     * @param  az		Index number of AZ angle to test.
     * @return false is there are no values above this threshold.
     */
	inline bool above_bounce_threshold(
			const wave_front* wave, size_t de, size_t az )
	{
		return  wave->bottom(de,az) > _max_bottom ||
				wave->surface(de,az) > _max_surface ||
				wave->caustic(de,az) > _max_caustic ||
				wave->upper(de,az) > _max_upper ||
				wave->lower(de,az) > _max_lower ;
	}

  private:

    /**
     * Any eigenray with a transmission loss intensity larger than
     * than this threshold is not sent the eigenray listeners.
     * Stored as a positive value in dB for later comparison
	 * with the positive eigenray.intensity value.
     */
    double _intensity_threshold;

    /**
     * Any eigenverb with a total power smaller than
     * than this threshold is not sent the eigenverb listeners.
     * Stored in linear units as a value between zero and one
     * for later comparison with the positive verb.power value.
     */
    double _eigenverb_threshold;

    /**
     * The maximum number of bottom bounces.
     * Any eigenray or eigenverb with more than this number
     * of bottom bounces is not sent the listeners.
     */
    int _max_bottom ;

    /**
     * The maximum number of surface bounces.
     * Any eigenray or eigenverb with more than this number
     * of surface bounces is not sent the listeners.
     */
    int _max_surface ;

    /**
     * The maximum number of caustic turning points.
     * Any eigenray or eigenverb with more than this number
     * of caustic turning points is not sent the listeners.
     */
    int _max_caustic ;

    /**
     * The maximum number of upper vertices.
     * Any eigenray or eigenverb with more than this number
     * of upper vertices is not sent the listeners.
     */
    int _max_upper ;

    /**
     * The maximum number of lower vertices.
     * Any eigenray or eigenverb with more than this number
     * of lower vertices is not sent the listeners.
     */
    int _max_lower ;

};

/// @}
}  // end of namespace waveq3d
}  // end of namespace usml
