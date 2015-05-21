/**
 * @file eigenray_interpolator.h
 * Interpolates eigenrays onto a new frequency axis.
 */
#pragma once

#include <usml/waveq3d/eigenray.h>
#include <usml/types/data_grid.h>

namespace usml {
namespace waveq3d {

using namespace usml::types ;

/// @ingroup waveq3d
/// @{

/**
 * Interpolates eigenrays onto a new frequency axis.
 * The envelope_generator repeatedly interpolates receiver eigenrays onto
 * the frequency axis of the source. This class encapsulates that functionality.
 */
class USML_DECLSPEC eigenray_interpolator {
public:

	/**
	 * Construct interpolating data_grid objects on original frequency scale.
	 *
	 * @param freq 		Original frequency axis for eigenrays.
	 * @param new_freq	Frequency axis for new eigenray.
	 */
	eigenray_interpolator(const seq_vector* freq, const seq_vector* new_freq);

	/**
	 * Destroy interpolating data_grid objects.
	 */
	~eigenray_interpolator() ;

	/**
	 * Interpolate frequency dependent terms onto a new frequency axis.
	 * Assumes that the calling routine had setup working space
	 * for the eigenray after interpolation.
	 *
	 * @param eigenrays         Eigenray List to be interpolated.
	 * @param new_eigenrays 	Eigenray List after interpolation.
	 */
	void interpolate( const eigenray_list& eigenrays,
	                    eigenray_list* new_eigenrays ) ;

private:
	size_t _freq_size ;
	const seq_vector* _new_freq ;
	data_grid<double,1>* _intensity_interp ;
	data_grid<double,1>* _phase_interp ;
};

/// @}
} // end of namespace waveq3d
} // end of namespace usml
