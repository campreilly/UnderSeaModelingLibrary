/**
 * @file eigenverb_interpolator.h
 * Interpolates eigenverbs onto a new frequency axis.
 */
#pragma once

#include <usml/eigenverb/eigenverb.h>
#include <usml/types/data_grid.h>

namespace usml {
namespace eigenverb {

using namespace usml::types ;

/// @ingroup eigenverb
/// @{

/**
 * Interpolates eigenverbs onto a new frequency axis.
 * The envelope_generator repeatedly interpolate receiver eigenverbs onto
 * the frequency axis of the source.  This class encapsulates that functionality.
 */
class USML_DECLSPEC eigenverb_interpolator {
public:

	/**
	 * Construct interpolating data_grid objects on original frequency scale.
	 *
	 * @param freq 		Original frequency axis for eigenverbs.
	 * @param new_freq	Frequency axis for new eigenverb.
	 */
	eigenverb_interpolator( const seq_vector* freq, const seq_vector* new_freq) ;

	/**
	 * Destroy interpolating data_grid objects.
	 */
	~eigenverb_interpolator() ;

	/**
	 * Interpolate frequency dependent terms onto a new frequency axis.
	 * Assumes that the calling routine had setup working space
	 * for the Eigenverb after interpolation, and that this working
	 *
	 * @param verb 		Eigenverb to be interpolated.
	 * @param new_verb 	Eigenverb after interpolation.
	 */
	void interpolate( const eigenverb& verb, eigenverb* new_verb ) ;

private:
	size_t _freq_size ;
	const seq_vector* _new_freq ;
	data_grid<double,1>* _energy_interp ;
	data_grid<double,1>*_length2_interp ;
	data_grid<double,1>*_width2_interp ;
};

/// @}
}// end of namespace eigenverb
} // end of namespace usml
