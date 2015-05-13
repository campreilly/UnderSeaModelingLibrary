/**
 * @file eigenray_interpolator.cc
 * Interpolates eigenrays onto a new frequency axis.
 */
#include <usml/waveq3d/eigenray_interpolator.h>

using namespace usml::waveq3d;

/**
 * Construct interpolating data_grid object on original frequency scale.
 */
eigenray_interpolator::eigenray_interpolator(
		const seq_vector* freq, const seq_vector* new_freq)
{
	_new_freq = new_freq;
	const seq_vector* ax[1] = { freq };
	_freq_size = freq->size();
	_intensity_interp = new data_grid<double, 1>(ax);
	_phase_interp = new data_grid<double, 1>(ax);
}

/**
 * Destroy interpolating data_grid objects.
 */
eigenray_interpolator::~eigenray_interpolator() {
	delete _intensity_interp;
	delete _phase_interp;
}

/**
 * Interpolate frequency dependent terms onto a new frequency axis.
 */
void eigenray_interpolator::interpolate(
		const eigenray_list& eigenrays, eigenray_list* new_eigenrays)
{
	// fill the interpolating data_grids with data

//	for (size_t f = 0; f < _freq_size; ++f) {
//		size_t index[1] = { f };
//		_intensity_interp->data(index, eigenrays.intensity[f]);
//		_phase_interp->data(index, eigenrays.phase[f]);
//	}
//
//	// copy terms that are not frequency dependent
//
//	new_eigenrays->time = eigenrays.time ;
//	new_eigenrays->source_de = eigenrays.source_de ;
//	new_eigenrays->source_az = eigenrays.source_az ;
//	new_eigenrays->target_de = eigenrays.target_de ;
//	new_eigenrays->target_az = eigenrays.target_az ;
//	new_eigenrays->surface = eigenrays.surface ;
//	new_eigenrays->bottom = eigenrays.bottom ;
//  new_eigenrays->caustic = eigenrays.caustic ;
//	new_eigenrays->upper = eigenrays.upper ;
//	new_eigenrays->lower = eigenrays.lower ;
//
//	// interpolate results to new frequency axis
//	// assume that calling routine has set new_eigenrays->freq
//
//	for (size_t f = 0; f < _new_freq->size(); ++f) {
//		double location[1] = { (*_new_freq)[f] };
//		new_eigenrays->intensity[f] = _intensity_interp->interpolate(location);
//		new_eigenrays->phase[f] = _phase_interp->interpolate(location);
//	}
}
