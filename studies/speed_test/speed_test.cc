/**
 * @file fspeed_test.cc
 */
#include <usml/waveq3d/waveq3d.h>
#include <usml/netcdf/netcdf_files.h>
#include <fstream>
#include <sys/time.h>

#define USML_DEBUG

using namespace usml::waveq3d ;
using namespace usml::netcdf ;

/**
 * Cubic generating function and its derivative
 * functions in the x and y directions.
 */

//static double cubic2d( double _val[] ) {
//    double func_val = ( _val[0]*_val[0]*_val[0] ) *
//            ( _val[1]*_val[1]*_val[1] ) ;
//    return func_val ;
//}

//static double deriv2d_x( double _val[] ) {
//    double func_val = 3.0 * ( _val[0]*_val[0] ) *
//            ( _val[1]*_val[1]*_val[1] ) ;
//    return func_val ;
//}
//
//static double deriv2d_y( double _val[] ) {
//    double func_val = 3.0 * ( _val[1]*_val[1] ) *
//            ( _val[0]*_val[0]*_val[0] ) ;
//    return func_val ;
//}

//static double cubic3d( double _val[] ) {
//    double func_val = ( _val[0]*_val[0]*_val[0] ) *
//            _val[1] * _val[2] ;
//    return func_val ;
//}
//
//static double deriv3d_x( double _val[] ) {
//    double func_val = ( _val[0]*_val[0]*_val[0] ) *
//            _val[2] ;
//    return func_val ;
//}
//
//static double deriv3d_y( double _val[] ) {
//    double func_val = ( _val[0]*_val[0]*_val[0] ) *
//            _val[1] ;
//    return func_val ;
//}
//static double deriv3d_z( double _val[] ) {
//    double func_val = 3.0 * ( _val[0]*_val[0] ) *
//            _val[1] * _val[2];
//    return func_val ;
//}

/**
 * Command line interface.
 */
int main() {
/** =====Test for speed on 2-Dimensions one location at a time===== */
//    cout << "=== datagrid_interpolation_speed_test_2d ===" << endl;
//
//    int num_points = 1e6 ;
//    int param = 5 ;
//    int counter = 0 ;
//    unsigned index[2] ;
//    double x_y[2] ;
//
//    seq_vector* ax[2] ;
//    ax[0] = new seq_linear(1.0, 1.0, 5) ;
//    ax[1] = new seq_linear(1.0, 1.0, 5) ;
//    data_grid<double,2>* grid = new data_grid<double,2>(ax);
//
//    for(int i = 0; i<2; ++i) {
////        grid->interp_type(i, GRID_INTERP_NEAREST);
////        grid->interp_type(i, GRID_INTERP_LINEAR);
//        grid->interp_type(i, GRID_INTERP_PCHIP);
//        grid->edge_limit(i, true);
//    }
//
//    for(int i=0; i < (*ax[0]).size(); ++i) {
//        for(int j=0; j < (*ax[1]).size(); ++j) {
//                index[0] = i ;
//                index[1] = j ;
//                x_y[0] = i + 1 ;
//                x_y[1] = j + 1 ;
//                double number = cubic2d(x_y) ;
//                grid->data( index, number ) ;
//        }
//    }
//
//    cout << "==========simple_data grid=============" << endl;
//    cout << "axis[0]: " << *ax[0] << endl;
//    cout << "axis[1]: " << *ax[1] << endl;
//    for(int i=0; i < (*ax[0]).size(); i++ ) {
//        for(int j=0; j < (*ax[1]).size(); j++ ) {
//            index[0] = i ;
//            index[1] = j ;
//            cout << grid->data(index) ;
//            (j < (*ax[1]).size()-1) ? cout << "\t" :  cout << endl;
//        }
//    }
//    cout << endl;
//
//    double spot[2] ;
//    matrix<double*> location (num_points,1) ;
//    for(int i=0; i<num_points; ++i) {
//        spot[0] = param * randgen::uniform();
//        spot[1] = param * randgen::uniform();
//        location(i,0) = spot ;
//    }
//
//    struct timeval time ;
//    struct timezone zone ;
//    gettimeofday( &time, &zone ) ;
//    double start = time.tv_sec + time.tv_usec * 1e-6 ;
//    while ( counter != num_points ) {
//        grid->interpolate( location(counter, 0) );
//        ++counter;
//    }
//    gettimeofday( &time, &zone ) ;
//    double complete = time.tv_sec + time.tv_usec * 1e-6 ;
//
//	cout << "Time to complete interpolation using data_grid method was "
//		 << (complete-start) << " sec." << endl;
//
//    data_grid_bathy* fast_grid = new data_grid_bathy(*grid, true);
//    counter = 0 ;
//    gettimeofday( &time, &zone ) ;
//    start = time.tv_sec + time.tv_usec * 1e-6 ;
//    while ( counter != num_points ) {
//        fast_grid->interpolate( location(counter, 0) );
//        ++counter;
//    }
//    gettimeofday( &time, &zone ) ;
//    complete = time.tv_sec + time.tv_usec * 1e-6 ;
//
//	cout << "Time to complete interpolation using fast_grid method was "
//		 << (complete-start) << " sec." << endl;

/** =====Test for speed on 3-Dimensions one location at a time===== */
//    cout << "=== datagrid_interpolation_speed_test_3d ===" << endl;
//
//    int num_points = 1e0 ;
//    int param = 5 ;
//    int counter = 0 ;
//    unsigned index[3] ;
//    double x_y_z[3] ;
//
//    seq_vector* ax[3] ;
//    ax[0] = new seq_linear(1.0, 1.0, 10) ;
//    ax[1] = new seq_linear(1.0, 1.0, 10) ;
//    ax[2] = new seq_linear(1.0, 1.0, 10) ;
//    data_grid<double,3>* grid = new data_grid<double,3>(ax);
//
//    grid->interp_type(0, GRID_INTERP_PCHIP);
//    grid->edge_limit(0, true);
//    grid->interp_type(1, GRID_INTERP_LINEAR);
//    grid->edge_limit(1, true);
//    grid->interp_type(2, GRID_INTERP_LINEAR);
//    grid->edge_limit(2, true);
//
//    for(int i=0; i < (*ax[0]).size(); ++i) {
//        for(int j=0; j < (*ax[1]).size(); ++j) {
//            for(int k=0; k < (*ax[2]).size(); ++k) {
//                    index[0] = i ;
//                    index[1] = j ;
//                    index[2] = k ;
//                    x_y_z[0] = i + 1 ;
//                    x_y_z[1] = j + 1 ;
//                    x_y_z[2] = k + 1 ;
//                    double number = cubic3d(x_y_z) ;
//                    grid->data( index, number ) ;
//            }
//        }
//    }
//
//    #ifdef USML_DEBUG
//        cout << "==========simple_data grid=============" << endl;
//        cout << "axis[0]: " << *ax[0] << endl;
//        cout << "axis[1]: " << *ax[1] << endl;
//        cout << "axis[2]: " << *ax[2] << endl;
//        for(int i=0; i < (*ax[0]).size(); i++ ) {
//            cout << "\t" << i << ",:,:" << endl;
//            for(int j=0; j < (*ax[1]).size(); j++ ) {
//                cout << "\t\t" ;
//                for(int k=0; k < (*ax[2]).size(); k++ ) {
//                    index[0] = i ;
//                    index[1] = j ;
//                    index[2] = k ;
//                    cout << grid->data(index) ;
//                    (k < (*ax[2]).size()-1) ? cout << "\t" :  cout << endl;
//                }
//            }
//            cout << endl;
//        }
//        cout << endl;
//    #endif
//
//    double spot[3] ;
//    matrix<double*> location (num_points,1) ;
//    for(int i=0; i<num_points; ++i) {
//        spot[0] = param * randgen::uniform();
//        spot[1] = param * randgen::uniform();
//        spot[2] = param * randgen::uniform();
//        location(i,0) = spot ;
//    }
//
//    struct timeval time ;
//    struct timezone zone ;
//    gettimeofday( &time, &zone ) ;
//    double start = time.tv_sec + time.tv_usec * 1e-6 ;
//    double derv[3] ;
//    while ( counter != num_points ) {
//        grid->interpolate( location(counter, 0), derv );
//        ++counter;
//    }
//    gettimeofday( &time, &zone ) ;
//    double complete = time.tv_sec + time.tv_usec * 1e-6 ;
//
//	cout << "Time to complete interpolation using data_grid method was "
//		 << (complete-start) << " sec." << endl;
//
//    data_grid_svp* fast_grid = new data_grid_svp(*grid, true);
//    counter = 0 ;
//    gettimeofday( &time, &zone ) ;
//    start = time.tv_sec + time.tv_usec * 1e-6 ;
//    while ( counter != num_points ) {
//        fast_grid->interpolate( location(counter, 0), derv );
//        ++counter;
//    }
//    gettimeofday( &time, &zone ) ;
//    complete = time.tv_sec + time.tv_usec * 1e-6 ;
//
//	cout << "Time to complete interpolation using fast_grid method was "
//		 << (complete-start) << " sec." << endl;

/** =====Test for speed on 3-Dimensions multiple locations at a time===== */
    cout << "=== datagrid_3d_speed_matrix_pass ===" << endl;

    int n_pts = 1e0 ;
    int m_pts = 1e0 ;
//    double param = 4 ;
    const double lat1 = 16.2 ;
    const double lat2 = 24.6 ;
    const double lng1 = -164.4;
    const double lng2 = -155.5 ;
    data_grid<double,3>* grid = new netcdf_profile( USML_STUDIES_DIR "/cmp_speed/std14profile.nc",
            0.0, lat1, lat2, lng1, lng2, wposition::earth_radius ) ;

    grid->interp_type(0, GRID_INTERP_PCHIP);
    grid->edge_limit(0, true);
    grid->interp_type(1, GRID_INTERP_LINEAR);
    grid->edge_limit(1, true);
    grid->interp_type(2, GRID_INTERP_LINEAR);
    grid->edge_limit(2, true);

    #ifdef USML_DEBUG
        unsigned index[3] ;
        double rad = 180.0 / M_PI ;
        const seq_vector* ax0 = grid->axis(0) ;
        const seq_vector* ax1 = grid->axis(1) ;
        const seq_vector* ax2 = grid->axis(2) ;
        cout << "==========complex_data grid=============" << endl;
        cout << "grid->axis0: (" << (*ax0)(0) - wposition::earth_radius << ", "
             << (*ax0)(1) - wposition::earth_radius << ", " << (*ax0)(2) - wposition::earth_radius << ")" << endl;
        cout << "increment(0): " << ax0->increment(0) << "\tincrement(1): " << ax0->increment(1) << endl;
        cout << "grid->axis1: (" << 90-(*ax1)(6)*rad << ", " << 90-(*ax1)(7)*rad << ", " << 90.0-(*ax1)(8)*rad << ", " << 90.0-(*ax1)(9)*rad << ")" << endl;
        cout << "grid->axis2: (" << rad*(*ax2)(13) << ", " << rad*(*ax2)(14) << ", " << rad*(*ax2)(15) << ", " << rad*(*ax2)(16) << ")" << endl;
        for(int i=0; i < 4; i++ ) {
            cout << "\t" << i << ",:,:" << endl;
            for(int j=7; j < 11; j++ ) {
                cout << "\t\t" ;
                for(int k=13; k < 17; k++ ) {
                    index[0] = i ;
                    index[1] = j ;
                    index[2] = k ;
                    cout << grid->data(index) ;
                    (k < 16) ? cout << "\t" :  cout << endl;
                }
            }
            cout << endl;
        }
        cout << endl;
    #endif

    wposition location (n_pts,m_pts) ;
    matrix<double> _ssp (n_pts,m_pts) ;
    wvector derv (n_pts,m_pts) ;
//    for(unsigned i=0; i<n_pts; ++i) {
//        for(unsigned j=0; j<m_pts; ++j) {
//            location.latitude( i, j, 17.0 + param * randgen::uniform() );
//            location.longitude( i, j, -157.3 - param * randgen::uniform() );
//            location.altitude( i, j, -100.0 + j*10.0 );
//        }
//    }
    location.latitude( 0, 0, 18.2 );
    location.longitude( 0, 0, -160.0 );
    location.altitude( 0, 0, -6.0 );
    cout << "location.altitude: " << location.altitude() << endl;

        //data_grid
    profile_model* ssp = new profile_grid<double,3>(grid) ;
    struct timeval time ;
    struct timezone zone ;
    cout << "\t===Starting speed test comparison===" << endl;
    gettimeofday( &time, &zone ) ;
    double start = time.tv_sec + time.tv_usec * 1e-6 ;

    ssp->sound_speed( location, &_ssp, &derv );

    gettimeofday( &time, &zone ) ;
    double complete = time.tv_sec + time.tv_usec * 1e-6 ;

	cout << "Time to complete interpolation using data_grid method was "
		 << (complete-start) << " sec." << endl;
    cout << "data_grid results: " << _ssp << endl;
    cout << "---data_grid derv---" << endl;
    for ( unsigned n1=0 ; n1 < n_pts ; ++n1 ) {
        cout << "\t" << n1 << ",:" << endl;
        for ( unsigned n2=0 ; n2 < m_pts ; ++n2 ) {
            if(n2!=0) {cout << " " ;}
            else {cout << "\t" ;}
            cout << "(" << derv.rho(n1,n2) << ")" ;
        }
        cout << endl;
    }

        //data_grid_svp
    data_grid_svp* fast_grid = new data_grid_svp(*grid, true);
    profile_model* ssp_fast = new profile_grid_fast(fast_grid) ;

    gettimeofday( &time, &zone ) ;
    start = time.tv_sec + time.tv_usec * 1e-6 ;

    ssp_fast->sound_speed( location, &_ssp, &derv );

    gettimeofday( &time, &zone ) ;
    complete = time.tv_sec + time.tv_usec * 1e-6 ;

	cout << "Time to complete interpolation using fast_grid method was "
		 << (complete-start) << " sec." << endl;
    cout << "fast_3d ssp: " << _ssp << endl;
    cout << "---fast_3d derv---" << endl;
    for ( unsigned n1=0 ; n1 < n_pts ; ++n1 ) {
        cout << "\t" << n1 << ",:" << endl;
        for ( unsigned n2=0 ; n2 < m_pts ; ++n2 ) {
            if(n2!=0) {cout << " " ;}
            else {cout << "\t" ;}
            cout << "(" << derv.rho(n1,n2) << ")" ;
        }
        cout << endl;
    }

/** =====Accuracy Testing===== */
//    double value ;
//    spot[0] = 0.352; spot[1] = 3.4587; spot[2] = 2.8453 ;
//    location(0,0) = spot ;
//    cout << "location: (" << spot[0] << "," << spot[1] << "," << spot[2] << ")" << endl;
//    value = grid->interpolate( location(counter, 0), derv );
//    cout << "data_grid:\t" << value << "\tderv: (" << derv[0]
//         << ", " << derv[1] << ", " << derv[2] << ")" << endl;
//    value = fast_grid->interpolate( location(counter, 0), derv );
//    cout << "fast_grid:\t" << value << "\tderv: (" << derv[0]
//         << ", " << derv[1] << ", " << derv[2] << ")" << endl;
//    cout << "true value:\t" << cubic3d(spot) << "\tderv: ("
//         << deriv3d_z(spot) << ", " << deriv3d_x(spot) << ", "
//         << deriv3d_y(spot) << ")" << endl;

}

