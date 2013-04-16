/**
 * @file spreading_hybrid_gaussian.cc
 * Spreading loss based on a hybrid Gaussian beam theory.
 */
#include <usml/waveq3d/spreading_hybrid_gaussian.h>

#define USML_WAVEQ3D_DEBUG_DE
//#define USML_WAVEQ3D_DEBUG_AZ

using namespace usml::waveq3d;

const double spreading_hybrid_gaussian::SPREADING_WIDTH = TWO_PI ;
const double spreading_hybrid_gaussian::OVERLAP = 2.0 ;
const double spreading_hybrid_gaussian::THRESHOLD = 1.002305238 ;

/**
 * Normalize each wavefront cell by the surface area it takes up
 * one meter from source.
 */
spreading_hybrid_gaussian::spreading_hybrid_gaussian(wave_queue& wave) :
    spreading_model( wave, wave._frequencies->size() ),
    _norm_de(wave.num_de()),
    _norm_az(wave.num_de(), wave.num_az()),
    _beam_width(wave._frequencies->size()),
    _intensity_de(wave._frequencies->size()),
    _intensity_az(wave._frequencies->size())
{
    for (unsigned d = 0; d < wave.num_de() - 1; ++d) {
        double de1 = to_radians(wave.source_de(d));
        double de2 = to_radians(wave.source_de(d + 1));
        _norm_de(d) = de2 - de1;
        for (unsigned a = 0; a < wave.num_az() - 1; ++a) {
            double az1 = to_radians(wave.source_az(a));
            double az2 = to_radians(wave.source_az(a + 1));
            _norm_az(d, a) = (sin(de2) - sin(de1)) * (az2 - az1) / _norm_de(d);
        }
        _norm_az(d, wave.num_az() - 1) = _norm_az(d, wave.num_az() - 2);
    }

    _norm_de(wave.num_de() - 1) = _norm_de(wave.num_de() - 2);
    for (unsigned a = 0; a < wave._source_az->size() - 1; ++a) {
        _norm_az(wave.num_de() - 1, a) = _norm_az(wave.num_de() - 2, a);
    }

    _norm_de /= sqrt(TWO_PI);
    _norm_az /= sqrt(TWO_PI);
}

/**
 * Estimate intensity as the product of Gaussian contributions
 * in the D/E and AZ directions.
 */
const vector<double>& spreading_hybrid_gaussian::intensity(
    const wposition1& location, unsigned de, unsigned az,
    const vector<double>& offset, const vector<double>& distance )
{
    // get sound speed at target

    matrix<double> sound_speed(1, 1);
    wposition loc(1, 1);
    loc.rho(0, 0, location.rho());
    loc.theta(0, 0, location.theta());
    loc.phi(0, 0, location.phi());
    _wave._ocean.profile().sound_speed(loc, &sound_speed);

    // convert frequency into spreading distance

    for (unsigned f = 0; f < _wave._frequencies->size(); ++f) {
        _spread(f) = SPREADING_WIDTH
                   * sound_speed(0, 0) / (*_wave._frequencies)(f) ;
    }
    _spread = element_prod(_spread,_spread) ;

    // compute Gaussian beam components in DE and AZ directions

    intensity_de(de, az, offset, distance);
    intensity_az(de, az, offset, distance);
    _intensity_de = element_prod(_intensity_de, _intensity_az);
    return _intensity_de;
}

/**
 * Summation of Gaussian beam contributions from all cells in the D/E direction.
 */
void spreading_hybrid_gaussian::intensity_de( unsigned de, unsigned az,
    const vector<double>& offset, const vector<double>& distance )
{
    #ifdef USML_WAVEQ3D_DEBUG_DE
        cout << "spreading_hybrid_gaussian::intensity_de:"
             << endl << "\ttime=" << _wave._time
             << " de(" << de << ")=" << (*_wave._source_de)(de)
             << " az(" << az << ")=" << (*_wave._source_az)(az)
             << " srf=" << _wave._curr->surface(de, az)
             << " btm=" << _wave._curr->bottom(de, az)
             << " cst=" << _wave._curr->caustic(de, az)
             << " offset=" << offset(1)
             << " distance=" << distance(1)
             << endl;
//        for ( unsigned d=0 ; d < _wave.num_de() ; ) {
//            cout << "\ton_edge=" ;
//            for ( int n=0 ; n < 30 && d < _wave.num_de(); ++n, ++d ) {
//                cout << " " << _wave._curr->on_edge(d,az) ;
//            }
//            cout << endl ;
//        }
    #endif

    // compute contribution from center cell

    int d = (int) de;
    double cell_width = width_de(d, az, offset);// half width of center cell
    const double initial_width = cell_width;    // save width for upper angles
    const double L = distance(1) ;              // D/E dist from nearest ray
    double cell_dist = L - cell_width ;         // dist from center of this cell
    _intensity_de = gaussian(cell_dist, cell_width, _norm_de(d));

    #ifdef USML_WAVEQ3D_DEBUG_DE
        cout << "\t** center" << endl
             << "\tde(" << d << ")=" << (*_wave._source_de)(d)
             << " cell_dist=" << cell_dist
             << " cell_width=" << cell_width
             << " beam_width=" << sqrt(_beam_width)
             << " norm=" << _norm_de(d)
             << " intensity=" << _intensity_de
             << endl
             << "\t** lower " << endl;
    #endif

    // contribution from DE angle one lower than central cell

    d = (int) de - 1;
    cell_width = width_de(d, az, offset);   // half width of this cell
    cell_dist = L + cell_width;             // dist from center of this cell
    _intensity_de += gaussian(cell_dist, cell_width, _norm_de(d));

    #ifdef USML_WAVEQ3D_DEBUG_DE
        cout << "\tde(" << d << ")=" << (*_wave._source_de)(d)
             << " cell_dist=" << cell_dist
             << " cell_width=" << cell_width
             << " beam_width=" << sqrt(_beam_width)
             << " norm=" << _norm_de(d)
             << " intensity=" << _intensity_de
             << endl;
    #endif

    // exit early if central rays have a tiny contribution

    if ( _intensity_de(0) < 1e-10 ) return;

    // contribution from other lower DE angles
    // stop after processing last entry in ray family
    // stop when lowest frequency PL changes by < threshold

    for (d = (int) de - 2; d >= 0; --d) {
        // compute distance to cell center and cell width

        cell_dist += cell_width;         // add half width of prev cell
        if( _wave._curr->on_edge(d+1,az) && _wave._curr->on_edge(d,az) ) {
            double cell_width1 = width_de(d+1, az, offset);
            double cell_width2 = width_de(d-1, az, offset);
            cell_width = ( cell_width1 + cell_width2 ) / 2.0;
            cell_dist += cell_width;
        }
        else {
            cell_width = width_de(d, az, offset);
            cell_dist += cell_width;
        }

        // compute propagation loss contribution of this cell

        const double old_tl = _intensity_de(0);

        _intensity_de += gaussian(cell_dist, cell_width, _norm_de(d));

        #ifdef USML_WAVEQ3D_DEBUG_DE
            cout << "\tde(" << d << ")=" << (*_wave._source_de)(d)
                 << " cell_dist=" << cell_dist
                 << " cell_width=" << cell_width
                 << " beam_width=" << sqrt(_beam_width)
                 << " norm=" << _norm_de(d)
                 << " intensity=" << _intensity_de
                 << endl;
        #endif
        if ( _intensity_de(0) / old_tl < THRESHOLD ) break;
    }

    // contribution from higher DE angles
    // stop after processing last entry in ray family
    // stop when lowest frequency PL changes by < threshold

    cell_width = initial_width;
    cell_dist = L - cell_width ;
    #ifdef USML_WAVEQ3D_DEBUG_DE
        cout << "\t** higher" << endl;
    #endif

    const int size = _wave._source_de->size() - 1;
    for (d = (int) de + 1; d < size; ++d) {

        // compute distance to cell center and cell width

        cell_dist -= cell_width;         // add half width of prev cell
        if( _wave._curr->on_edge(d+1,az) && _wave._curr->on_edge(d,az) ) {
            double cell_width1 = width_de(d-1, az, offset);
            double cell_width2 = width_de(d+2, az, offset);
            cell_width = ( cell_width1 + cell_width2 ) / 2.0;
            cell_dist -= cell_width;
        }
        else {
            cell_width = width_de(d, az, offset);
            cell_dist -= cell_width;
        }
//        cell_dist -= cell_width;         // remove half width of this cell

        // compute propagation loss contribution of this cell

        const double old_tl = _intensity_de(0);
        _intensity_de += gaussian(cell_dist, cell_width, _norm_de(d));

        #ifdef USML_WAVEQ3D_DEBUG_DE
            cout << "\tde(" << d << ")=" << (*_wave._source_de)(d)
                 << " cell_dist=" << cell_dist
                 << " cell_width=" << cell_width
                 << " beam_width=" << sqrt(_beam_width)
                 << " norm=" << _norm_de(d)
                 << " intensity=" << _intensity_de
                 << endl;
        #endif
        if ( _intensity_de(0) / old_tl < THRESHOLD ) break;
    }
}

/**
 * Summation of Gaussian beam contributions from all cells in the AZ direction.
 */
void spreading_hybrid_gaussian::intensity_az( unsigned de, unsigned az,
    const vector<double>& offset, const vector<double>& distance )
{
    #ifdef USML_WAVEQ3D_DEBUG_AZ
        cout << "spreading_hybrid_gaussian::intensity_az:"
             << endl << "\ttime=" << _wave._time
             << " de(" << de << ")=" << (*_wave._source_de)(de)
             << " az(" << az << ")=" << (*_wave._source_az)(az)
             << " srf=" << _wave._curr->surface(de, az)
             << " btm=" << _wave._curr->bottom(de, az)
             << " cst=" << _wave._curr->caustic(de, az)
             << " offset=" << offset(2)
             << " distance=" << distance(2)
             << endl;
    #endif

    // compute contribution from center cell

    int a = (int) az;
    double cell_width = width_az(de, a, offset);// half width of center cell
    const double initial_width = cell_width;    // save width for upper angles
    const double L = distance(2) ;              // AZ dist from nearest ray
    double cell_dist = L - cell_width ;         // dist from center of this cell
    _intensity_az = gaussian(cell_dist, cell_width, _norm_az(de, a));

    #ifdef USML_WAVEQ3D_DEBUG_AZ
        cout << "\t** center" << endl
             << "\taz(" << a << ")=" << (*_wave._source_az)(a)
             << " cell_dist=" << cell_dist
             << " cell_width=" << cell_width
             << " beam_width=" << sqrt(_beam_width)
             << " norm=" << _norm_az(de,a)
             << " intensity=" << _intensity_az
             << endl
             << "\t** lower " << endl;
    #endif

    // contribution from AZ angle one lower than central cell

    a = (int) az - 1;
    cell_width = width_az(de, a, offset);   // half width of this cell
    cell_dist = L + cell_width;             // dist from center of this cell
    _intensity_az += gaussian(cell_dist, cell_width, _norm_az(de, a));

    #ifdef USML_WAVEQ3D_DEBUG_AZ
        cout << "\taz(" << a << ")=" << (*_wave._source_az)(a)
             << " cell_dist=" << cell_dist
             << " cell_width=" << cell_width
             << " beam_width=" << sqrt(_beam_width)
             << " norm=" << _norm_az(de,a)
             << " intensity=" << _intensity_az
             << endl;
    #endif

    // exit early if central rays have a tiny contribution

    if ( _intensity_az(0) < 1e-10 ) return;

    // contribution from other lower DE angles
    // stop after processing last entry in ray family
    // stop when lowest frequency PL changes by < threshold

    for (a = (int) az - 2; a >= 0; --a) {
        if ( _wave._curr->on_edge(de,a) ) break ;

        // compute distance to cell center and cell width

        cell_dist += cell_width;         // add half width of prev cell
        cell_width = width_az(de, a, offset);
        cell_dist += cell_width;         // add half width of this cell

        // compute propagation loss contribution of this cell

        const double old_tl = _intensity_az(0);
        _intensity_az += gaussian(cell_dist, cell_width, _norm_az(de, a));

        #ifdef USML_WAVEQ3D_DEBUG_AZ
            cout << "\taz(" << a << ")=" << (*_wave._source_az)(a)
                 << " cell_dist=" << cell_dist
                 << " cell_width=" << cell_width
                 << " beam_width=" << sqrt(_beam_width)
                 << " norm=" << _norm_az(de,a)
                 << " intensity=" << _intensity_az
                 << endl;
        #endif
        if ( _intensity_az(0) / old_tl < THRESHOLD ) break;
    }

    // contribution from higher DE angles
    // stop after processing last entry in ray family
    // stop when lowest frequency PL changes by < threshold

    cell_width = initial_width;
    cell_dist = L - cell_width ;
    #ifdef USML_WAVEQ3D_DEBUG_AZ
        cout << "\t** higher" << endl
             << "\tdist=" << cell_dist
             << " width=" << sqrt(_beam_width) << endl ;
    #endif
    const int size = _wave._source_az->size() - 1;
    for (a = (int) az + 1; a < size; ++a) {
        if ( _wave._curr->on_edge(de,a) ) break ;

        // compute distance to cell center and cell width

        cell_dist -= cell_width;         // remove half width of prev cell
        cell_width = width_az(de, a, offset);
        cell_dist -= cell_width;         // remove half width of this cell

        // compute propagation loss contribution of this cell

        const double old_tl = _intensity_az(0);
        _intensity_az += gaussian(cell_dist, cell_width, _norm_az(de, a));

        #ifdef USML_WAVEQ3D_DEBUG_AZ
            cout << "\taz(" << a << ")=" << (*_wave._source_az)(a)
                 << " cell_dist=" << cell_dist
                 << " cell_width=" << cell_width
                 << " beam_width=" << sqrt(_beam_width)
                 << " norm=" << _norm_az(de,a)
                 << " intensity=" << _intensity_az
                 << endl;
        #endif
        if ( _intensity_az(0) / old_tl < THRESHOLD ) break;
    }
}

/**
 * Interpolate the half-width of a cell in the D/E direction.
 */
double spreading_hybrid_gaussian::width_de(
    unsigned de, unsigned az, const vector<double>& offset )
{
    double L1, L2, length1, length2 ;

    // compute relative offsets in time (u) and azimuth (v)

    const double u = fabs(offset(0)) / _wave._time_step;
    const double v = fabs(offset(2)) / (*_wave._source_az).increment(az);

    // compute the DE width for the current time step
    //      L1 = cell width from DE to DE+1 along AZ
    //      L2 = cell width from DE to DE+1 along AZ+1
    //      length1 = current distance interpolated across AZ angles
    //      treat a nearly zero AZ offset as a special case
    //      create temporary wvector1 variables on the fly

    const wposition& pos1 = _wave._curr->position;
    L1 = wvector1(pos1,de,az).distance( wvector1(pos1,de+1,az) );
    if ( v < 1e-10 ) {
        length1 = L1 ;
    } else {
        L2 = wvector1(pos1,de,az+1).distance( wvector1(pos1,de+1,az+1) );
        length1 = (1.0-v) * L1 + v * L2;
    }

    // treat nearly zero time offset as a special case

    if ( u < 1e-10 ) return 0.5 * length1 ;

    // compute the DE width for the next time step
    //      L1 = cell width from DE to DE+1 along AZ
    //      L2 = cell width from DE to DE+1 along AZ+1
    //      length2 = next distance interpolated across AZ angles
    //      if time offset < zero, use previous instead of next wavefront
    //      treat a nearly zero AZ offset as a special case
    //      create temporary wvector1 variable on the fly

    const wposition& pos2 = (offset(0) < 0.0)
        ? _wave._prev->position
        : _wave._next->position;
    L1 = wvector1(pos2,de,az).distance( wvector1(pos2,de+1,az) );
    if ( v < 1e-10 ) {
        length2 = L1 ;
    } else {
        L2 = wvector1(pos2,de,az+1).distance( wvector1(pos2,de+1,az+1) );
        length2 = (1.0-v) * L1 + v * L2;
    }

    // interpolate across times

    return 0.5 * ( (1.0-u) * length1 + u * length2 ) ;
}

/**
 * Interpolate the half-width of a cell in the AZ direction.
 */
double spreading_hybrid_gaussian::width_az(
    unsigned de, unsigned az, const vector<double>& offset )
{
    double L1, L2, length1, length2 ;

    // compute relative offsets in time (u) and D/E (v)

    const double u = fabs(offset(0)) / _wave._time_step;
    const double v = fabs(offset(1)) / (*_wave._source_de).increment(de);

    // compute the AZ width for the current time step
    //      L1 = cell width from AZ to AZ+1 along DE
    //      L2 = cell width from AZ to AZ+1 along DE+1
    //      length1 = current distance interpolated across DE angles
    //      treat a nearly zero AZ offset as a special case
    //      create temporary wvector1 variables on the fly

    const wposition& pos1 = _wave._curr->position;
    L1 = wvector1(pos1, de, az).distance( wvector1(pos1, de, az+1) );
    if ( v < 1e-10 ) {
        length1 = L1 ;
    } else {
        L2 = wvector1(pos1, de+1, az).distance( wvector1(pos1, de+1, az+1) );
        length1 = (1.0-v) * L1 + v * L2;
    }

    // treat nearly zero time offset as a special case

    if ( u < 1e-10 ) return 0.5 * length1 ;

    // compute the AZ width for the next time step
    //      L1 = cell width from AZ to AZ+1 along DE
    //      L2 = cell width from AZ to AZ+1 along DE+1
    //      length2 = next distance interpolated across DE angles
    //      if time offset < zero, use previous instead of next wavefront
    //      treat a nearly zero AZ offset as a special case
    //      create temporary wvector1 variables on the fly

    const wposition& pos2 = (offset(0) < 0.0)
        ? _wave._prev->position
        : _wave._next->position;
    L1 = wvector1(pos2, de, az).distance( wvector1(pos2, de, az+1) );
    if ( v < 1e-10 ) {
        length2 = L1 ;
    } else {
        L2 = wvector1(pos2, de+1, az).distance( wvector1(pos2, de+1, az+1) );
        length2 = (1.0-v) * L1 + v * L2;
    }

    // interpolate across times

    return 0.5 * ( (1.0-u) * length1 + u * length2 ) ;
}
