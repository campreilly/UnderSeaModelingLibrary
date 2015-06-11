/**
 * @file eigenray_collection.cc
 * List of targets and their associated propagation data.
 */
#include <usml/waveq3d/eigenray_collection.h>
#include <netcdfcpp.h>

using namespace usml::waveq3d ;

/**
 * Constructor
 * Initialize the acoustic propagation effects associated with each target.
 * Initialize with references to wave front information.
 */
eigenray_collection::eigenray_collection(
    const seq_vector& frequencies,
    const wposition1& source_pos,
    const seq_vector& source_de,
    const seq_vector& source_az,
    double time_step,
    const wposition* targets )
        :
        _targets( targets ),
        _frequencies(frequencies.clone()),
        _source_pos(source_pos),
        _source_de (source_de.clone()),
        _source_az (source_az.clone()),
        _time_step(time_step),
        _eigenrays( size1(), size2() ),
        _num_eigenrays(0),
        _loss( size1(), size2() )
{
	initialize();
}

/**
 * Initialize with references to wave front information.
 */

void eigenray_collection::initialize()
{
    for ( size_t t1=0 ; t1 < _targets->size1() ; ++t1 ) {
        for ( size_t t2=0 ; t2 < _targets->size2() ; ++t2 ) {
            _loss(t1,t2).intensity.resize( _frequencies->size() ) ;
            _loss(t1,t2).intensity.clear() ;
            _loss(t1,t2).phase.resize( _frequencies->size() ) ;
            _loss(t1,t2).phase.clear() ;

        }
    }
}

/**
 * Compute propagation loss summed over all eigenrays.
 */
void eigenray_collection::sum_eigenrays( bool coherent ) {
    if( coherent ) {                                                             /// coherent signal summation
        for ( size_t t1=0 ; t1 < _targets->size1() ; ++t1 ) {
            for ( size_t t2=0 ; t2 < _targets->size2() ; ++t2 ) {

                double time = 0.0 ;
                double source_de = 0.0 ;
                double source_az_x = 0.0 ; // East/West component
                double source_az_y = 0.0 ; // North/South component
                double target_de = 0.0 ;
                double target_az_x = 0.0 ; // East/West component
                double target_az_y = 0.0 ; // North/South component
                int surface = -1 ;
                int bottom = -1 ;
                int caustic = -1 ;
                double wgt = 0.0 ;
                double max_a = 0.0 ;

                // compute at each frequency

                const eigenray_list* entry = eigenrays(t1,t2) ;
                eigenray* loss = &( _loss(t1,t2) ) ;

                    for ( size_t f=0 ; f < _frequencies->size() ; ++f ) {

                        // sum complex amplitudes over eigenrays

                        std::complex<double> phasor( 0.0, 0.0 ) ;

                        for ( eigenray_list::const_iterator iter = entry->begin() ;
                              iter != entry->end() ; ++iter )
                        {
                            const eigenray& ray = *iter ;

                            // complex pressure

                            double a = pow( 10.0, ray.intensity(f) / -20.0 ) ;                // pressure
                            double p = TWO_PI * (*_frequencies)(f) * ray.time + ray.phase(f) ;
                            p = fmod( p, TWO_PI ) ; // large phases bad for cos,sin
                            std::complex<double> value( a * cos(p), a * sin(p) ) ;
                            phasor += value ;

                            // other eigenray terms

                            a *= a ;          // scale by the pressure squared
                            wgt += a ;
                            time += a * ray.time ;
                            source_de += a * ray.source_de ;
                            source_az_x += a * sin(to_radians(ray.source_az)) ;
                            source_az_y += a * cos(to_radians(ray.source_az)) ;
                            target_de += a * ray.target_de ;
                            target_az_x += a * sin(to_radians(ray.target_az)) ;
                            target_az_y += a * cos(to_radians(ray.target_az)) ;
                            if ( a > max_a ) {
                                max_a = a ;
                                surface = ray.surface ;
                                bottom = ray.bottom ;
                                caustic = ray.caustic ;
                            }
                        } // end eigenray_list

                        // convert back into intensity (dB) and phase (radians) values

                        loss->intensity(f) = -20.0*log10( max(1e-15,abs(phasor)) ) ;
                        loss->phase(f) = arg(phasor) ;
                    } // end frequency

                // weighted average of other eigenray terms

                loss->time = time / wgt ;
                loss->source_de = source_de / wgt ;
                loss->source_az = 90.0 - to_degrees(atan2(source_az_y, source_az_x)) ;
                loss->target_de = target_de / wgt ;
                loss->target_az = 90.0 - to_degrees(atan2(target_az_y, target_az_x)) ;
                loss->surface = surface ;
                loss->bottom = bottom ;
                loss->caustic = caustic ;
            } // end target size2
        } // end target size1
    } else {                                                                    /// incoherent signal summation
        for ( size_t t1=0 ; t1 < _targets->size1() ; ++t1 ) {
            for ( size_t t2=0 ; t2 < _targets->size2() ; ++t2 ) {

                double time = 0.0 ;
                double source_de = 0.0 ;
                double source_az_x = 0.0 ; // East/West component
                double source_az_y = 0.0 ; // North/South component
                double target_de = 0.0 ;
                double target_az_x = 0.0 ; // East/West component
                double target_az_y = 0.0 ; // North/South component
                int surface = -1 ;
                int bottom = -1 ;
                int caustic = -1 ;
                double wgt = 0.0 ;
                double max_a = 0.0 ;

                // compute at each frequency

                const eigenray_list* entry = eigenrays(t1,t2) ;
                eigenray* loss = &( _loss(t1,t2) ) ;

                    for ( size_t f=0 ; f < _frequencies->size() ; ++f ) {

                        // sum complex amplitudes over eigenrays

                        double phasor = 0.0 ;

                        for ( eigenray_list::const_iterator iter = entry->begin() ;
                              iter != entry->end() ; ++iter )
                        {
                            const eigenray& ray = *iter ;

                            // complex pressure

                            const double a = pow( 10.0, ray.intensity(f) / -10.0 ) ;        // pressure squared
                            phasor += a ;

                            // other eigenray terms

                            wgt += a ;
                            time += a * ray.time ;
                            source_de += a * ray.source_de ;
                            source_az_x += a * sin(to_radians(ray.source_az)) ;
                            source_az_y += a * cos(to_radians(ray.source_az)) ;
                            target_de += a * ray.target_de ;
                            target_az_x += a * sin(to_radians(ray.target_az)) ;
                            target_az_y += a * cos(to_radians(ray.target_az)) ;
                            if ( a > max_a ) {
                                max_a = a ;
                                surface = ray.surface ;
                                bottom = ray.bottom ;
                                caustic = ray.caustic ;
                            }
                        } // end eigenray_list

                        // convert back into intensity (dB) and phase (radians) values

                        loss->intensity(f) = -20.0*log10( max(1e-15,sqrt(phasor)) ) ;
                        loss->phase(f) = 0.0 ;
                    } // end frequency

                // weighted average of other eigenray terms

                loss->time = time / wgt ;
                loss->source_de = source_de / wgt ;
                loss->source_az = 90.0 - to_degrees(atan2(source_az_y, source_az_x)) ;
                loss->target_de = target_de / wgt ;
                loss->target_az = 90.0 - to_degrees(atan2(target_az_y, target_az_x)) ;
                loss->surface = surface ;
                loss->bottom = bottom ;
                loss->caustic = caustic ;
            } // end targets size2
        } // end targets size1
    } // end coherent/incoherent
}

/**
 * Add eigenray via eigenray_listener
 */
void eigenray_collection::add_eigenray(
		size_t target_row, size_t target_col, eigenray ray, size_t runID )
{
	 _eigenrays(target_row, target_col).push_back( ray ) ;
	 ++_num_eigenrays ;
}

/**
 * Write eigenray_collection data to to netCDF file.
 */
void eigenray_collection::write_netcdf( const char* filename, const char* long_name )
{
    NcFile* nc_file = new NcFile(filename, NcFile::Replace);
    if (long_name) {
        nc_file->add_att("long_name", long_name);
    }
    nc_file->add_att("Conventions", "COARDS");

    // dimensions

    NcDim *freq_dim = nc_file->add_dim("frequency", (long) _frequencies->size());
	NcDim *row_dim = nc_file->add_dim("rows", (long) _targets->size1());
	NcDim *col_dim = nc_file->add_dim("cols", (long) _targets->size2());
    NcDim *eigenray_dim = nc_file->add_dim("eigenrays",
           (long) ( _num_eigenrays + _loss.size1() * _loss.size2()) ) ;
	NcDim *launch_de_dim = nc_file->add_dim("launch_de", (long) _source_de->size());
	NcDim *launch_az_dim = nc_file->add_dim("launch_az", (long) _source_az->size());

    // coordinates

    NcVar *src_lat_var = nc_file->add_var("source_latitude", ncDouble);
    NcVar *src_lng_var = nc_file->add_var("source_longitude", ncDouble);
    NcVar *src_alt_var = nc_file->add_var("source_altitude", ncDouble);
    NcVar *launch_de_var = nc_file->add_var("launch_de", ncDouble, launch_de_dim);
    NcVar *launch_az_var = nc_file->add_var("launch_az", ncDouble, launch_az_dim);
    NcVar *time_step_var = nc_file->add_var("time_step", ncDouble);
    NcVar *freq_var = nc_file->add_var("frequency", ncDouble, freq_dim);

    NcVar *latitude_var = nc_file->add_var("latitude", ncDouble, row_dim, col_dim);
    NcVar *longitude_var = nc_file->add_var("longitude", ncDouble, row_dim, col_dim);
    NcVar *altitude_var = nc_file->add_var("altitude", ncDouble, row_dim, col_dim);

    NcVar *proploss_index_var = nc_file->add_var("proploss_index", ncLong, row_dim, col_dim);
    NcVar *eigenray_index_var = nc_file->add_var("eigenray_index", ncLong, row_dim, col_dim);
    NcVar *eigenray_num_var = nc_file->add_var("eigenray_num", ncLong, row_dim, col_dim);

    NcVar *intensity_var = nc_file->add_var("intensity", ncDouble, eigenray_dim, freq_dim);
    NcVar *phase_var = nc_file->add_var("phase", ncDouble, eigenray_dim, freq_dim);
    NcVar *time_var = nc_file->add_var("travel_time", ncDouble, eigenray_dim);
    NcVar *source_de_var = nc_file->add_var("source_de", ncDouble, eigenray_dim);
    NcVar *source_az_var = nc_file->add_var("source_az", ncDouble, eigenray_dim);
    NcVar *target_de_var = nc_file->add_var("target_de", ncDouble, eigenray_dim);
    NcVar *target_az_var = nc_file->add_var("target_az", ncDouble, eigenray_dim);
    NcVar *surface_var = nc_file->add_var("surface", ncShort, eigenray_dim);
    NcVar *bottom_var = nc_file->add_var("bottom", ncShort, eigenray_dim);
    NcVar *caustic_var = nc_file->add_var("caustic", ncShort, eigenray_dim);

    // units

    src_lat_var->add_att("units", "degrees_north");
    src_lng_var->add_att("units", "degrees_east");
    src_alt_var->add_att("units", "meters");
    src_alt_var->add_att("positive", "up");
    launch_de_var->add_att("units", "degrees");
    launch_de_var->add_att("positive", "up");
    launch_az_var->add_att("units", "degrees_true");
    launch_az_var->add_att("positive", "clockwise");
    time_step_var->add_att("units", "seconds");
    freq_var->add_att("units", "hertz");

    latitude_var->add_att("units", "degrees_north");
    longitude_var->add_att("units", "degrees_east");
    altitude_var->add_att("units", "meters");
    altitude_var->add_att("positive", "up");

    proploss_index_var->add_att("units", "count");
    eigenray_index_var->add_att("units", "count");
    eigenray_num_var->add_att("units", "count");

    intensity_var->add_att("units", "dB");
    phase_var->add_att("units", "radians");
    time_var->add_att("units", "seconds");

    source_de_var->add_att("units", "degrees");
    source_de_var->add_att("positive", "up");
    source_az_var->add_att("units", "degrees_true");
    source_az_var->add_att("positive", "clockwise");

    target_de_var->add_att("units", "degrees");
    target_de_var->add_att("positive", "up");
    target_az_var->add_att("units", "degrees_true");
    target_az_var->add_att("positive", "clockwise");

    surface_var->add_att("units", "count");
    bottom_var->add_att("units", "count");
    caustic_var->add_att("units", "count");

    // write source parameters

    double v;
    v = _source_pos.latitude();
    src_lat_var->put(&v);
    v = _source_pos.longitude();   src_lng_var->put(&v);
    v = _source_pos.altitude();    src_alt_var->put(&v);
    for (size_t d = 0; d < _source_de->size(); ++d) {
        launch_de_var->set_cur((long)d);
        v = (*_source_de)(d);       launch_de_var->put(&v, 1);
    }
    for (size_t a = 0; a < _source_az->size(); ++a) {
        launch_az_var->set_cur((long)a);
        v = (*_source_az)(a);       launch_az_var->put(&v, 1);
    }
    v = _time_step;
    time_step_var->put(&v);
    freq_var->put(vector<double>(*_frequencies).data().begin(), (long) _frequencies->size());

    // write target coordinates

    latitude_var->put(_targets->latitude().data().begin(),
            (long) _targets->size1(), (long) _targets->size2());
    longitude_var->put(_targets->longitude().data().begin(),
		(long)_targets->size1(), (long) _targets->size2());
    altitude_var->put(_targets->altitude().data().begin(),
		(long)_targets->size1(), (long) _targets->size2());

    // write propagation loss and eigenrays to disk

    int record = 0; // current record number
    for (long t1 = 0; t1 < (long) _targets->size1(); ++t1) {
        for (long t2 = 0; t2 < (long) _targets->size2(); ++t2) {
            int num = (long) _eigenrays(t1, t2).size();
            proploss_index_var->set_cur(t1, t2);
            eigenray_index_var->set_cur(t1, t2);
            eigenray_num_var->set_cur(t1, t2);

            proploss_index_var->put(&record, 1, 1);  // 1st rec = summed PL
            int next_rec = record+1 ;
            eigenray_index_var->put(&next_rec, 1, 1); // followed by list of rays
            eigenray_num_var->put(&num, 1, 1);

            eigenray_list::const_iterator iter = _eigenrays(t1, t2).begin();

            for (int n = -1; n < num; ++n) {

                // set record number for each eigenray data element

                intensity_var->set_cur(record);
                phase_var->set_cur(record);
                time_var->set_cur(record);
                source_de_var->set_cur(record);
                source_az_var->set_cur(record);
                target_de_var->set_cur(record);
                target_az_var->set_cur(record);
                surface_var->set_cur(record);
                bottom_var->set_cur(record);
                caustic_var->set_cur(record);
                ++record ;

                // case 1 : write propagation loss summed over all eigenrays

                if (n < 0) {
                    const eigenray& loss = _loss(t1, t2);
                    intensity_var->put(loss.intensity.data().begin(),
                            1, (long) _frequencies->size());
                    phase_var->put(loss.phase.data().begin(),
                            1, (long) _frequencies->size());
                    time_var->put(&loss.time, 1);
                    source_de_var->put(&loss.source_de, 1);
                    source_az_var->put(&loss.source_az, 1);
                    target_de_var->put(&loss.target_de, 1);
                    target_az_var->put(&loss.target_az, 1);
                    surface_var->put(&loss.surface, 1);
                    bottom_var->put(&loss.bottom, 1);
                    caustic_var->put(&loss.caustic, 1);

                // case 2 : write individual eigenray

                } else {
                    const eigenray& loss = *iter++;
                    intensity_var->put(loss.intensity.data().begin(),
                            1, (long) _frequencies->size());
                    phase_var->put(loss.phase.data().begin(),
                            1, (long) _frequencies->size());
                    time_var->put(&loss.time, 1);
                    source_de_var->put(&loss.source_de, 1);
                    source_az_var->put(&loss.source_az, 1);
                    target_de_var->put(&loss.target_de, 1);
                    target_az_var->put(&loss.target_az, 1);
                    surface_var->put(&loss.surface, 1);
                    bottom_var->put(&loss.bottom, 1);
                    caustic_var->put(&loss.caustic, 1);

                } // if sum or individual
            }   // loop over # of eigenrays
        } // loop over target# t2
    } // loop over target# t1

    // close file

    delete nc_file; // destructor frees all netCDF temp variables
}
