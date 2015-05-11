/**
 * @file eigenverb_interpolator.cc
 * Interpolates eigenverbs onto a new frequency axis.
 */
#include <usml/eigenverb/eigenverb_interpolator.h>

using namespace usml::eigenverb;

/**
 * Construct interpolating data_grid object on original frequency scale.
 */
eigenverb_interpolator::eigenverb_interpolator(
		const seq_vector* freq, const seq_vector* new_freq)
{
	_new_freq = new_freq;
	const seq_vector* ax[1] = { freq };
	_freq_size = freq->size();
	_energy_interp = new data_grid<double, 1>(ax);
	_length2_interp = new data_grid<double, 1>(ax);
	_width2_interp = new data_grid<double, 1>(ax);
}

/**
 * Destroy interpolating data_grid objects.
 */
eigenverb_interpolator::~eigenverb_interpolator() {
	delete _energy_interp;
	delete _length2_interp;
	delete _width2_interp;
}

/**
 * Interpolate frequency dependent terms onto a new frequency axis.
 */
void eigenverb_interpolator::interpolate(
		const eigenverb& verb, eigenverb* new_verb)
{
	// fill the interpolating data_grids with data

	for (size_t f = 0; f < _freq_size; ++f) {
		size_t index[1] = { f };
		_energy_interp->data(index, verb.energy[f]);
		_length2_interp->data(index, verb.length2[f]);
		_width2_interp->data(index, verb.width2[f]);
	}

	// copy terms that are not frequency dependent

	new_verb->time = verb.time ;
	new_verb->position = verb.position ;
	new_verb->direction = verb.direction ;
	new_verb->grazing = verb.grazing ;
	new_verb->sound_speed = verb.sound_speed ;
	new_verb->de_index = verb.de_index ;
	new_verb->az_index = verb.az_index ;
	new_verb->surface = verb.surface ;
	new_verb->bottom = verb.bottom ;
	new_verb->upper = verb.upper ;
	new_verb->lower = verb.lower ;

	// interpolate results to new frequency axis
	// assume that calling routine has set new_verb->freq

	for (size_t f = 0; f < _new_freq->size(); ++f) {
		double location[1] = { (*_new_freq)[f] };
		new_verb->energy[f] = _energy_interp->interpolate(location);
		new_verb->length2[f] = _length2_interp->interpolate(location);
		new_verb->width2[f] = _width2_interp->interpolate(location);
	}
}
