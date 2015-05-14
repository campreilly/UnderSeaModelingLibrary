/**
 * @file eigenray_interpolator.cc
 * Interpolates eigenrays onto a new frequency axis.
 */
#include <usml/waveq3d/eigenray_interpolator.h>
#include <boost/foreach.hpp>

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

    std::list<eigenray>::iterator new_eigenray_list_iter;
    new_eigenray_list_iter = new_eigenrays->begin();

    BOOST_FOREACH (eigenray ray, eigenrays) {
        for (size_t f = 0; f < _freq_size; ++f) {
            size_t index[1] = { f };
            _intensity_interp->data(index, ray.intensity[f]);
            _phase_interp->data(index, ray.phase[f]);
        }
        // copy terms that are not frequency dependent
        eigenray new_ray = *new_eigenray_list_iter;

        new_ray.time = ray.time ;
        new_ray.source_de = ray.source_de ;
        new_ray.source_az = ray.source_az ;
        new_ray.target_de = ray.target_de ;
        new_ray.target_az = ray.target_az ;
        new_ray.surface = ray.surface ;
        new_ray.bottom = ray.bottom ;
        new_ray.caustic = ray.caustic ;
        new_ray.upper = ray.upper ;
        new_ray.lower = ray.lower ;

        // interpolate results to new frequency axis
        // assume that calling routine has set new_eigenrays->freq

        for (size_t f = 0; f < _new_freq->size(); ++f) {
            double location[1] = { (*_new_freq)[f] };
            new_ray.intensity[f] = _intensity_interp->interpolate(location);
            new_ray.phase[f] = _phase_interp->interpolate(location);
        }
        ++new_eigenray_list_iter;
        new_eigenrays->push_back(new_ray);
    }
    // Remove incoming data from new eigenray list
    for (size_t i = 0; i < eigenrays.size(); ++i) {
        new_eigenrays->pop_front();
    }
}
