/**
 * @file spreading_hybrid_gaussian.cc
 * Spreading loss based on a hybrid Gaussian beam theory.
 */
#include <usml/waveq3d/spreading_hybrid_gaussian.h>

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
    _intensity_az(wave._frequencies->size()),
    _duplicate(wave.num_az(), 1)
{
    for (size_t d = 0; d < wave.num_de() - 1 ; ++d) {
        double de1 = to_radians(wave.source_de(d));
        double de2 = to_radians(wave.source_de(d + 1));
        _norm_de(d) = de2 - de1;
        for (size_t a = 0; a < wave.num_az() - 1; ++a) {
            double az1 = to_radians(wave.source_az(a));
            double az2 = to_radians(wave.source_az(a + 1));
            _norm_az(d, a) = (sin(de2) - sin(de1)) * (az2 - az1) / _norm_de(d);
        }
        _norm_az(d, wave.num_az() - 1) = _norm_az(d, 0);
    }

    _norm_de(wave.num_de() - 1) = _norm_de(0);
    for (size_t a = 0; a < wave.num_az() ; ++a) {
        _norm_az(wave.num_de() - 1, a) = _norm_az(0, a);
    }

    _norm_de /= sqrt(TWO_PI);
    _norm_az /= sqrt(TWO_PI);

//    cout << "_norm_az: " << _norm_az << endl;
}

/**
 * Estimate intensity as the product of Gaussian contributions
 * in the D/E and AZ directions.
 */
const vector<double>& spreading_hybrid_gaussian::intensity(
    const wposition1& location, size_t de, size_t az,
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

    for (size_t f = 0; f < _wave._frequencies->size(); ++f) {
        _spread(f) = SPREADING_WIDTH
                   * sound_speed(0, 0) / (*_wave._frequencies)(f) ;
    }
    _spread = element_prod(_spread,_spread) ;

    // compute Gaussian beam components in DE and AZ directions

    vector<double> corrected_offset = offset ;
    std::size_t a ;
    if( offset(2) < 0.0 ) {
        if( (int)(az-1) < 0 ) {
            a = _wave._source_az->size() - 2 ;
        } else { a = az - 1 ; }
        corrected_offset(2) = corrected_offset(2) + 1 ;
    } else { a = az ; }
    std::size_t d ;
    if( offset(1) < 0.0 ) {
        d = de - 1 ;
        corrected_offset(1) = corrected_offset(1) + 1 ;
    } else {
        d = de ;
    }
    intensity_de(de, a, corrected_offset, distance) ;
    intensity_az(d, az, corrected_offset, distance);
    _intensity_de = element_prod(_intensity_de, _intensity_az);
    return _intensity_de;
}

/**
 * Summation of Gaussian beam contributions from all cells in the D/E direction.
 */
void spreading_hybrid_gaussian::intensity_de( size_t de, size_t az,
    const vector<double>& offset, const vector<double>& distance )
{
    // compute contribution from center cell
    double temp ;                                   // used to check for crossing wave families
    double s=1.0 ;
    int d = (int) de ;
    if ( d == _wave.num_de() - 1 ) --d ;
    double cell_width = width_de(d, az, offset) ;   // half width of center cell
    if( abs(cell_width) > 80 ) {s=5.0;}
    const double initial_width = cell_width ;      // save width for upper angles
    const double L = distance(1) ;                 // D/E dist from nearest ray
    double cell_dist = L - cell_width ;            // dist from center of this cell
    _intensity_de = s*gaussian(cell_dist, cell_width, _norm_de(d));

    // contribution from DE angle one lower than central cell

    d = (int) de - 1;
    cell_width = width_de(d, az, offset);   // half width of this cell
    cell_dist = L + cell_width;             // dist from center of this cell
    _intensity_de += gaussian(cell_dist, cell_width, _norm_de(d));

    // exit early if central rays have a tiny contribution

    if ( _intensity_de(0) < 1e-10 ) return;

    // contribution from other lower DE angles
    // stop after processing last entry in ray family
    // stop when lowest frequency PL changes by < threshold

    for (d = (int) de - 2; d >= 0; --d) {
        // compute distance to cell center and cell width

        temp = cell_dist;                // assign for a temp check
        cell_dist += cell_width;         // add half width of prev cell
        if( _wave._curr->on_edge(d+1,az) && _wave._curr->on_edge(d,az) ) {
            if( _wave._curr->caustic(d+1,az) != _wave._curr->caustic(d,az) ) break;
            cell_dist += cell_width;
        }
        else {
            cell_width = width_de(d, az, offset);
            cell_dist += cell_width;
            if( abs(temp) > abs(cell_dist) ) break;
        }

        // compute propagation loss contribution of this cell

        const double old_tl = _intensity_de(0);
        if( _wave._curr->caustic(d,az) != 0 && s!=1.0 ) {s=0.25;}
        _intensity_de += s*gaussian(cell_dist, cell_width, _norm_de(d));

        if ( _intensity_de(0) / old_tl < THRESHOLD ) break;
    }

    // contribution from higher DE angles
    // stop after processing last entry in ray family
    // stop when lowest frequency PL changes by < threshold

    cell_width = initial_width;
    if( abs(cell_width) > 80 ) {s=5.0;}
    cell_dist = L - cell_width ;

    const size_t size = _wave._source_de->size() - 1 ;
    for (d = (int) de + 1; d < size; ++d) {

        // compute distance to cell center and cell width

        temp = cell_dist;                // assign for a temp check
        cell_dist -= cell_width;         // add half width of prev cell
        if ( _wave._curr->on_edge(d+1,az) && _wave._curr->on_edge(d,az) ) {
            if ( _wave._curr->caustic(d+1,az) != _wave._curr->caustic(d,az) ) break;
            cell_dist -= cell_width ;
        }
        else {
            cell_width = width_de(d, az, offset);
            cell_dist -= cell_width;
            if( abs(temp) > abs(cell_dist) ) break;
        }

        // compute propagation loss contribution of this cell

        const double old_tl = _intensity_de(0);
        _intensity_de += s*gaussian(cell_dist, cell_width, _norm_de(d));

        if ( _intensity_de(0) / old_tl < THRESHOLD ) break;
    }
}

/**
 * Summation of Gaussian beam contributions from all cells in the AZ direction.
 */
void spreading_hybrid_gaussian::intensity_az( size_t de, size_t az,
    const vector<double>& offset, const vector<double>& distance )
{
    // compute contribution from center cell
    size_t az_upper, az_lower ;
    double az_first = abs((*_wave._source_az)(0)) ;
    double az_last = abs((*_wave._source_az)(_wave._source_az->size()-1)) ;
    double boundary_check = az_first + az_last ;
    if ( boundary_check == 360.0 &&
        ( fmod(az_first, 360.0) == fmod(az_last, 360.0) ) )
    { az_lower = az_upper = az ; }
    else { az_lower = 0 ; az_upper = _wave._source_az->size()-2 ; }
    const size_t size = _wave._source_az->size() - 1 ;

    _duplicate.clear() ;
    size_t a = az;
    _duplicate(a,0) = true ;
    double cell_width = width_az(de, a, offset);// half width of center cell
    const double initial_width = cell_width;    // save width for upper angles
    const double L = distance(2) ;              // AZ dist from nearest ray
    double cell_dist = L - cell_width ;         // dist from center of this cell
    double _new_norm ;
    if ( de >= _wave._source_de->size() - 2 ) { _new_norm = _norm_az(1,a) ; }
    else { _new_norm = _norm_az(de,a) ; }
    _intensity_az = gaussian(cell_dist, cell_width, _new_norm);

    // contribution from AZ angle one lower than central cell

    if( (az-1) < 0) {a = size - 1 ;}
    else {a = (az - 1) ;}
    _duplicate(a,0) = true ;
    cell_width = width_az(de, a, offset);   // half width of this cell
    cell_dist = L + cell_width;             // dist from center of this cell
    if ( de >= _wave._source_de->size() - 2 ) { _new_norm = _norm_az(1,a) ; }
    else { _new_norm = _norm_az(de,a) ; }
    _intensity_az += gaussian(cell_dist, cell_width, _new_norm);

    // exit early if central rays have a tiny contribution

    if ( _intensity_az(0) < 1e-10 ) return;

    // contribution from other lower AZ angles
    // stop after processing last entry in ray family
    // stop when lowest frequency PL changes by < threshold

    if( (a-1) < 0) {a = size - 1 ;}
    else { --a ;}
    while ( a%size != az_lower ) {
        if ( _duplicate(a,0) ) break ;
        _duplicate(a,0) = true ;
        if ( _wave._curr->on_edge(de,a) ) break ;

        // compute distance to cell center and cell width

        cell_dist += cell_width;         // add half width of prev cell
        cell_width = width_az(de, a, offset);
        cell_dist += cell_width;         // add half width of this cell

        // compute propagation loss contribution of this cell

        const double old_tl = _intensity_az(0);
        if ( de >= _wave._source_de->size() - 2 ) { _new_norm = _norm_az(1,a) ; }
        else { _new_norm = _norm_az(de,a) ; }
        _intensity_az += gaussian(cell_dist, cell_width, _new_norm);

        if ( _intensity_az(0) / old_tl < THRESHOLD ) break;
        if ( a == 0 ) { a += size - 1 ; }
        else { --a ;}
    }

    // contribution from higher DE angles
    // stop after processing last entry in ray family
    // stop when lowest frequency PL changes by < threshold

    cell_width = initial_width;
    cell_dist = L - cell_width ;

    a = az + 1 ;
    while ( a%size != az_upper ) {
        if ( a == size ) { a = 0 ;}
        if ( _duplicate(a,0) ) break ;
        _duplicate(a,0) = true ;
        if ( _wave._curr->on_edge(de,a) ) break ;

        // compute distance to cell center and cell width

        cell_dist -= cell_width;         // remove half width of prev cell
        cell_width = width_az(de, a, offset);
        cell_dist -= cell_width;         // remove half width of this cell

        // compute propagation loss contribution of this cell

        const double old_tl = _intensity_az(0);
        if ( de >= _wave._source_de->size() - 2 ) { _new_norm = _norm_az(1,a) ; }
        else { _new_norm = _norm_az(de,a) ; }
        _intensity_az += gaussian(cell_dist, cell_width, _new_norm);

        if ( _intensity_az(0) / old_tl < THRESHOLD ) break;
        ++a ;
    }
}

/**
 * Interpolate the half-width of a cell in the D/E direction.
 */
double spreading_hybrid_gaussian::width_de(
    size_t de, size_t az, const vector<double>& offset )
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

    const size_t size = _wave._source_az->size() - 1 ;
    size_t az_wrap ;
    if ( az+1 >= size ) {az_wrap = 0 ; }
    else { az_wrap = az + 1 ; }
    const wposition& pos1 = _wave._curr->position;
    L1 = wvector1(pos1,de,az).distance( wvector1(pos1,de+1,az) );
    if ( v < 1e-10 ) {
        length1 = L1 ;
    } else {
        L2 = wvector1(pos1,de,az_wrap).distance( wvector1(pos1,de+1,az_wrap) );
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
        L2 = wvector1(pos2,de,az_wrap+1).distance( wvector1(pos2,de+1,az_wrap+1) );
        length2 = (1.0-v) * L1 + v * L2;
    }

    // interpolate across times

    return 0.5 * ( (1.0-u) * length1 + u * length2 ) ;
}

/**
 * Interpolate the half-width of a cell in the AZ direction.
 */
double spreading_hybrid_gaussian::width_az(
    size_t de, size_t az, const vector<double>& offset )
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
    const size_t size = _wave._source_az->size() - 1 ;
    const size_t size_de = _wave._source_de->size() - 1 ;
    size_t az_wrap ;
    size_t de_max = de ;
    if ( de+1 >= size_de ) { de_max = size_de - 2 ; }
    else { de_max = de ; }
    if ( az+1 > size ) {az_wrap = 0 ;}
    else { az_wrap = az + 1 ;}
    L1 = wvector1(pos1, de, az).distance( wvector1(pos1, de, az_wrap) );
    if ( v < 1e-10 || abs(v - 1.0) < 1e-10 ) {
        length1 = L1 ;
    } else {
        L2 = wvector1(pos1, de_max+1, az).distance( wvector1(pos1, de_max+1, az_wrap) );
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
    L1 = wvector1(pos2, de, az).distance( wvector1(pos2, de, az_wrap) );
    if ( v < 1e-10 || abs(v - 1.0) < 1e-10 ) {
        length2 = L1 ;
    } else {
        L2 = wvector1(pos2, de_max+1, az).distance( wvector1(pos2, de_max+1, az_wrap) );
        length2 = (1.0-v) * L1 + v * L2;
    }

    // interpolate across times
    return 0.5 * ( (1.0-u) * length1 + u * length2 ) ;
}
