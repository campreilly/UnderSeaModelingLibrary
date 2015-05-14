/**
 * @file wavefront_generator.cc
 * Generates eigenrays and eigenverbs for the reverberation model.
 */

#define PRINTOUT_WAVE_DATA
//#define NO_EIGENVERBS

#include <usml/eigenverb/wavefront_generator.h>

using namespace usml::eigenverb ;

int wavefront_generator::number_de = 181;
int wavefront_generator::number_az = 18;
double wavefront_generator::time_maximum = 90.0;         // sec
double wavefront_generator::time_step = 0.01;            // sec
double wavefront_generator::intensity_threshold = 300.0; // dB
int wavefront_generator::max_bottom = 999 ;
int wavefront_generator::max_surface = 999 ;

/**
 * Constructor
 */
wavefront_generator::wavefront_generator(shared_ptr<ocean_model> ocean,
        wposition1 source_position, const wposition* target_positions,
        const seq_vector* frequencies, wavefront_listener* listener,
        double vertical_beamwidth,  double depression_elevation_angle, int run_id)
    : _done(false),
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
 * Default Constructor
 */
wavefront_generator::wavefront_generator()
    : _done(false),
      _run_id(0),
      _number_de(number_de),
      _number_az(number_az),
      _time_maximum(time_maximum),
      _time_step(time_step),
      _source_position(NULL),
      _target_positions(NULL),
      _frequencies(NULL),
      _wavefront_listener(NULL)
{
    _ocean.reset();
}

void wavefront_generator::run()
{
    // For Matlab output
    std::string ncname_wave = "./generator_wave.nc";
    std::string ncname_proploss = "./generator_proploss.nc";
    std::string ncname_eigenverbs = "./generator_eigenverbs_";
#ifdef PRINTOUT_WAVE_DATA
    bool print_out = true;
#else
    bool print_out = false;
#endif

    /** Pointer to Proploss object */
    eigenray_collection* proploss = NULL;

    /** Pointer to eigenverbs object */
    eigenverb_collection* eigenverbs = NULL;

    // check to see if WaveQ3D propagation model task has already been aborted

    if ( _abort ) {
        cout << id() << " WaveQ3D   *** aborted before execution ***" << endl ;
        return ;
    }


    // Setup DE sequence rayfan for WaveQ3D
    // Augment rayfan with additional de's near -90 and 90.

    // TODO - add augmented DE
    //size_t num_xtra_rays = 6;
    //seq_rayfan de(-90.0, 90.0, _number_de - num_xtra_rays);
    //seq_augment aug_de(&de, num_xtra_rays);
    //cout << std::setprecision(8);
    //cout << "aug_de: " << aug_de << endl ;

    seq_rayfan de(-90.0, 90.0, _number_de);

    seq_linear az(0.0, 180.0, _number_az, true);

    if (_target_positions != NULL) {
        proploss = new eigenray_collection(*(_frequencies), _source_position, de, az, _time_step, _target_positions);
    }

	wave_queue wave(*(_ocean.get()), *(_frequencies),
			_source_position, de, az,
			_time_step, _target_positions, _run_id);

    if ( proploss != NULL ) {
        wave.add_eigenray_listener(proploss);
    }

#ifndef NO_EIGENVERBS

    eigenverbs = new eigenverb_collection( _ocean.get()->num_volume());
    wave.add_eigenverb_listener(eigenverbs);

#endif

    wave.intensity_threshold(intensity_threshold);
    wave.max_bottom(max_bottom);
	wave.max_surface(max_surface);

    if (print_out)
    {
        // Plot the wavefront.
        wave.init_netcdf(ncname_wave.c_str());
        wave.save_netcdf();
    }

    // propagate wavefront & record

    while (wave.time() < _time_maximum)
    {
        wave.step();
        if (print_out)
        {
            wave.save_netcdf();
        }
    }

    if (print_out)
    {
        wave.close_netcdf();
    }

    if (proploss != NULL) {
        proploss->sum_eigenrays();
    }

    if (print_out) {
        if (proploss != NULL) {
            proploss->write_netcdf(ncname_proploss.c_str());
        }
        if (eigenverbs != NULL){
            for ( int n=0 ; n < eigenverbs->num_interfaces() ; ++n ) {
                std::ostringstream filename ;
                switch (n)
                {
                    case 0:
                        filename << ncname_eigenverbs.c_str() << "bottom.nc" ;
                        break;
                    case 1:
                        filename << ncname_eigenverbs.c_str() << "surface.nc" ;
                        break;
                    default:
                        filename << ncname_eigenverbs.c_str() << n << ".nc" ;
                }
                eigenverbs->write_netcdf( filename.str().c_str(),n) ;
            }
        }
    }

    if (proploss != NULL) {
        eigenray_collection::reference rays(proploss);
        _wavefront_listener->update_eigenrays(rays);
    }

    if (eigenverbs != NULL) {
        eigenverb_collection::reference verbs(eigenverbs);
        _wavefront_listener->update_eigenverbs(verbs);
    }

    // mark task as complete
    _done = true ;
}


