/**
 * @file wavefront_generator.cc
 * Generates eigenrays and eigenverbs for the reverberation model.
 */

//#define PRINTOUT_WAVE_DATA
//#define NO_EIGENVERBS
#include <usml/eigenverb/wavefront_generator.h>

using namespace usml::eigenverb;

int wavefront_generator::number_de = 181;
int wavefront_generator::number_az = 18;
int wavefront_generator::extra_rays = 4;
double wavefront_generator::time_maximum = 90.0;         // sec
double wavefront_generator::time_step = 0.01;            // sec
double wavefront_generator::intensity_threshold = 300.0; // dB
int wavefront_generator::max_bottom = 999;
int wavefront_generator::max_surface = 999;

/**
 * Construct wavefront generator from the data items needed to run WaveQ3D.
 */
wavefront_generator::wavefront_generator(shared_ptr<ocean_model> ocean,
	wposition1 source_position, const wposition* target_positions,
	const seq_vector* frequencies, wavefront_listener* listener,
	double vertical_beamwidth, double depression_elevation_angle,
	int run_id)
	:
	_done(false),
	_run_id(run_id),
	_number_de(number_de),
	_number_az(number_az),
	_time_maximum(time_maximum),
	_time_step(time_step),
	_source_position(source_position),
	_target_positions(target_positions),
	_frequencies(frequencies),
	_ocean(ocean),
	_wavefront_listener(listener)
{
}

/**
 * Executes the WaveQ3D propagation model.
 */
void wavefront_generator::run() {

	// check to see if task has already been aborted

	if (_abort) {
		cout << id() << " WaveQ3D   *** aborted before execution ***" << endl;
		return;
	}

	// initialize wavefront

	seq_rayfan orig_de(-90.0, 90.0, _number_de);
	seq_augment de(&orig_de, extra_rays);

	double az_increment = 360.0 / _number_az;
	seq_linear az(0.0, az_increment, 359.9);

	wave_queue wave(
		*(_ocean.get()), *(_frequencies), _source_position, de, az,
		_time_step, _target_positions, _run_id);
	wave.intensity_threshold(intensity_threshold);
	wave.max_bottom(max_bottom);
	wave.max_surface(max_surface);

	// create listener to store eigenrays

	eigenray_collection::reference eigenrays ;
	if ( _target_positions ) {
		eigenrays.reset( new eigenray_collection(
			*_frequencies, _source_position,
			de, az, _time_step, _target_positions) ) ;
		wave.add_eigenray_listener(eigenrays.get());
	}

	// create listener to store eigenverbs

	eigenverb_collection::reference eigenverbs(
			new eigenverb_collection(_ocean.get()->num_volume()) ) ;
	wave.add_eigenverb_listener( eigenverbs.get() );

	// propagate wavefront to build eigenrays and eigenverbs

	while (wave.time() < _time_maximum) {
		wave.step();
		if (_abort) {
			cout << id() << " WaveQ3D   *** aborted during execution ***" << endl;
			return;
		}
	}
	if ( eigenrays != NULL ) eigenrays->sum_eigenrays();

	// distribute eigenrays and eigenverbs to sensor pairs

	_wavefront_listener->update_wavefront_data(eigenrays, eigenverbs);
	_done = true;
}

