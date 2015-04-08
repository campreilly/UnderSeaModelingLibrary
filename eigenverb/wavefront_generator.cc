/**
 * @file wavefront_generator.cc
 *
 *  Created on: Mar 25, 2015
 *      Author: Ted Burns, AEgis Technologies Group, Inc.
 */

#include <usml/waveq3d/eigenray_collection.h>
#include <usml/eigenverb/wavefront_generator.h>

using namespace usml::eigenverb ;

int wavefront_generator::number_de = 181;
int wavefront_generator::number_az = 18;
double wavefront_generator::pulse_length = 0.5; // sec
double wavefront_generator::time_step = 0.1; // sec
double wavefront_generator::time_maximum = 90.0; // sec

/**
 * Default Constructor
 */
wavefront_generator::wavefront_generator()
    : _runID(0),
      _done(false),
      _range_maximum(0.0),
      _intensity_threshold(300.0),
      _depression_elevation_angle(0.0),
      _vertical_beamwidth(0.0),
      _frequencies(NULL),
      _wavefront_listener(NULL)
{
    _ocean.reset();
}

void wavefront_generator::run()
{
    // For Matlab output
    const char* ncname_wave = "./generator_wave.nc";
    const char* ncname_proploss = "./generator_proploss.nc";
#ifdef USML_DEBUG
    bool print_out = true;
#else
    bool print_out = false;
#endif

    /** Pointer to Proploss object */
    eigenray_collection* proploss;

    // check to see if WaveQ3D propagation model task has already been aborted

    if ( _abort ) {
        cout << id() << " WaveQ3D   *** aborted before execution ***" << endl ;
        return ;
    }

    // Setup DE sequence vector for WaveQ3D
//    double de_start = _depression_elevation_angle - (_vertical_beamwidth * 0.5f);
//    double de_end = _depression_elevation_angle + (_vertical_beamwidth * 0.5f);
//
//    // Don't go above 90 or below -90
//    // Add extra 2 degrees on each end
//    de_start = max(de_start - 2.0, -90.0);
//    de_end = min(de_end + 2.0, 90.00);


    seq_rayfan de(-90.0, 90.0, number_de);

    seq_linear az(0.0, 180.0, number_az, true);


    proploss = new eigenray_collection(*(_frequencies), _sensor_position, de, az, time_step, &_targets);

    wave_queue wqWave(*(_ocean.get()), *(_frequencies), _sensor_position, de, az, time_step, &_targets, _runID,
                                                                usml::waveq3d::wave_queue::HYBRID_GAUSSIAN);

    wqWave.add_eigenray_listener(proploss);

    wqWave.intensity_threshold(_intensity_threshold);

    if (print_out)
    {
        // Plot the rays.
        wqWave.init_netcdf(ncname_wave);
        wqWave.save_netcdf();
    }
    // propagate rays & record
    while (wqWave.time() < time_maximum)
    {
        wqWave.step();
        if (print_out)
        {
            wqWave.save_netcdf();
        }
    }

    if (print_out)
    {
        wqWave.close_netcdf();
    }

    proploss->sum_eigenrays();

    if (print_out) {
        proploss->write_netcdf(ncname_proploss);
    }

    eigenray_collection::reference rays(proploss);
    _wavefront_listener->update_eigenrays(rays);

    // mark task as complete
    _done = true ;
}


