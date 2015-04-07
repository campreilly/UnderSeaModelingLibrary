/**
 * @file spreading_hybrid_gaussian.cc
 * Spreading loss based on a hybrid Gaussian beam theory.
 */
#include <usml/waveq3d/spreading_hybrid_gaussian.h>

//#define DEBUG_EIGENRAYS
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
    for (size_t d = 0; d < wave.num_de() - 1; ++d) {
        double de1 = to_radians(wave.source_de(d));
        double de2 = to_radians(wave.source_de(d + 1));
        _norm_de(d) = de2 - de1;
        for (size_t a = 0; a < wave.num_az() - 1; ++a) {
            double az1 = to_radians(wave.source_az(a));
            double az2 = to_radians(wave.source_az(a + 1));
            _init_area(d,a) = (sin(de2) - sin(de1)) * (az2 - az1) ;
            _norm_az(d,a) =  _init_area(d,a) / _norm_de(d);
        }
        _norm_az(d, wave.num_az() - 1) = _norm_az(d, wave.num_az() - 2);
    }

    _norm_de(wave.num_de() - 1) = _norm_de(wave.num_de() - 2);
    size_t n = wave.num_de() - 1 ;
    size_t m = wave.num_de() - 2 ;
    for (size_t a = 0; a < wave._source_az->size() - 1; ++a) {
    	_init_area(n,a) = _init_area(m,a) ;
        _norm_az(n,a) = _norm_az(m,a);
    }

    _norm_de /= sqrt(TWO_PI);
    _norm_az /= sqrt(TWO_PI);
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

    size_t d = de, a = az ;
    vector<double> new_offset = offset ;

    // Preserve offset of the AZ dimension and accumulate the correct
    // gaussian contributions in the DE dimension by correcting
    // the distance and offsets.

    if( offset(2) < 0.0 ) {
		if( az > 0 ) {
			a = az - 1 ;
		} else {
			if( _wave._az_boundary ) {
				a = _wave._source_az->size() - 2 ;
			}
		}
    	new_offset(2) = offset(2) + _wave._source_az->increment(a) ;
    }
	intensity_de(de, a, new_offset, distance) ;

    // Preserve offset of the DE dimension and accumulate the correct
    // gaussian contributions in the AZ dimension by correcting
    // the distance and offsets.

    new_offset = offset ;
    if( offset(1) < 0.0 && !_wave._curr->on_edge(de-1,az) ) {
    	d = de - 1 ;
    	new_offset(1) = offset(1) + _wave._source_de->increment(d) ;
    }
    intensity_az(d, az, new_offset, distance) ;
    _intensity_de = element_prod(_intensity_de, _intensity_az) ;
    return _intensity_de;
}

/**
 * Summation of Gaussian beam contributions from all cells in the D/E direction.
 */
void spreading_hybrid_gaussian::intensity_de( size_t de, size_t az,
    const vector<double>& offset, const vector<double>& distance )
{
    #ifdef DEBUG_EIGENRAYS
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
    #endif

    // compute contribution from center cell

    int d = (int) de ;
    double cell_width = width_de(d, az, offset) ;// half width of center cell
    const double initial_width = cell_width ;    // save for upper angles
    const double L = distance(1) ;               // D/E dist from nearest ray
    double cell_dist = L - cell_width ;          // dist from center of this cell
    _intensity_de = gaussian(cell_dist, cell_width, _norm_de(d)) ;

    #ifdef DEBUG_EIGENRAYS
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


    d = (int) de - 1 ;
    cell_width = width_de(d, az, offset) ;   // half width of this cell
    cell_dist = L + cell_width ;             // dist from center of this cell
    _intensity_de += gaussian(cell_dist, cell_width, _norm_de(d)) ;

    #ifdef DEBUG_EIGENRAYS
        cout << "\tde(" << d << ")=" << (*_wave._source_de)(d)
             << " cell_dist=" << cell_dist
             << " cell_width=" << cell_width
             << " beam_width=" << sqrt(_beam_width)
             << " norm=" << _norm_de(d)
             << " intensity=" << _intensity_de
             << endl;
    #endif


    if( _intensity_de(0) < 1e-10 ) return ;

    // contribution from other lower DE angles
    // stop after processing last entry in ray family
    // stop when lowest frequency PL changes by < threshold

    for(d = (int) de - 2; d >= 0; --d) {
    	bool virtual_ray = _wave._curr->on_edge(d+1,az) && _wave._curr->on_edge(d,az) ;
    	double _new_norm ;				// corrected normalization when using a virtual ray
        cell_dist += cell_width ;       // add half width of prev cell

        // compute propagation loss contribution of this cell
        // virtual rays use previous cell's cell_width

        if( virtual_ray ) {
            cell_dist += cell_width ;
            _new_norm = _norm_de(d+1) ;
        }
        else {
            cell_width = width_de(d, az, offset) ;
            cell_dist += cell_width ;
            _new_norm = _norm_de(d) ;
        }

        const double old_tl = _intensity_de(0) ;

        _intensity_de += gaussian(cell_dist, cell_width, _new_norm) ;

        #ifdef DEBUG_EIGENRAYS
            cout << "\tde(" << d << ")=" << (*_wave._source_de)(d)
                 << " cell_dist=" << cell_dist
                 << " cell_width=" << cell_width
                 << " beam_width=" << sqrt(_beam_width)
                 << " norm=" << _norm_de(d)
                 << " intensity=" << _intensity_de
                 << endl;
        #endif
        if ( _intensity_de(0) / old_tl < THRESHOLD ) break;
        if( virtual_ray ) break ;
    }

    // contribution from higher DE angles
    // stop after processing last entry in ray family
    // stop when lowest frequency PL changes by < threshold

    #ifdef DEBUG_EIGENRAYS
        cout << "\t** higher" << endl ;
    #endif

    cell_width = initial_width;
    cell_dist = L - cell_width ;

    const size_t size = (size_t)_wave._source_de->size() - 1 ;
    for(d = (int) de + 1; d < size; ++d) {
        bool virtual_ray = _wave._curr->on_edge(d+1,az) && _wave._curr->on_edge(d,az) ;
    	double _new_norm ;				// corrected normalization when using a virtual ray
        cell_dist -= cell_width ;		// add half width of prev cell

        // compute propagation loss contribution of this cell
        // virtual rays use previous cell's cell_width


        if( virtual_ray ) {
            cell_dist -= cell_width ;
            _new_norm = _norm_de(d-1) ;
        }
        else {
            cell_width = width_de(d, az, offset) ;
            cell_dist -= cell_width ;
            _new_norm = _norm_de(d) ;
        }

        const double old_tl = _intensity_de(0) ;
        _intensity_de += gaussian(cell_dist, cell_width, _new_norm) ;

        #ifdef DEBUG_EIGENRAYS
            cout << "\tde(" << d << ")=" << (*_wave._source_de)(d)
                 << " cell_dist=" << cell_dist
                 << " cell_width=" << cell_width
                 << " beam_width=" << sqrt(_beam_width)
                 << " norm=" << _norm_de(d)
                 << " intensity=" << _intensity_de
                 << endl;
        #endif
        if ( _intensity_de(0) / old_tl < THRESHOLD ) break;
        if( virtual_ray ) break ;
    }
}

/**
 * Summation of Gaussian beam contributions from all cells in the AZ direction.
 */
void spreading_hybrid_gaussian::intensity_az( size_t de, size_t az,
    const vector<double>& offset, const vector<double>& distance )
{
    // compute contribution from center cell
    size_t az_upper, az_lower, max_az, max_de ;
    max_de = _wave._source_de->size() - 2 ;			// Maximum allowed DE
    max_az = _wave._source_az->size() - 1 ;			// Maximum index in AZ

    // Check for an az branch point condition and set the upper and lower
    // AZ indices appropriately

    if( _wave._az_boundary ) {
    	az_lower = az_upper = az ;
    } else {
    	az_lower = 0 ;
    	az_upper = max_az - 1 ;
    }

    // Clear duplicate rays
    _duplicate.clear() ;
    size_t a = az ;
    _duplicate(a,0) = true ;
    double cell_width = width_az(de, a, offset) ;	// half width of center cell
    const double initial_width = cell_width ;    	// save width for upper angles
    const double L = distance(2) ;              	// AZ dist from nearest ray
    double cell_dist = L - cell_width ;         	// dist from center of this cell
    double _new_norm ;

    // Check for an abnormal normalization constant, ie when DE is close to a de branch pt

    if( de >= max_de ) _new_norm = _norm_az(1,a) ;
    else _new_norm = _norm_az(de,a) ;
    _intensity_az = gaussian(cell_dist, cell_width, _new_norm) ;

    // contribution from AZ angle one lower than central cell

    if( az < 1 ) a = max_az - 1 ;
    else a = az - 1 ;
    _duplicate(a,0) = true ;
    cell_width = width_az(de, a, offset) ;   // half width of this cell
    cell_dist = L + cell_width ;             // dist from center of this cell

    // Check for an abnormal normalization constant, ie when DE is close to a de branch pt

    if( de >= max_de ) _new_norm = _norm_az(1,a) ;
    else _new_norm = _norm_az(de,a) ;
    _intensity_az += gaussian(cell_dist, cell_width, _new_norm) ;

    // exit early if central rays have a tiny contribution

    if( _intensity_az(0) < 1e-10 ) return;

    // contribution from other lower AZ angles
    // stop after processing last entry in ray family
    // stop if the ray is a duplicate
    // stop when lowest frequency PL changes by < threshold

    if( a < 1 ) a = max_az - 1 ;
    else --a ;
    while( (a % max_az) != az_lower ) {
        if( _duplicate(a,0) ) break ;
        _duplicate(a,0) = true ;
        if( _wave._curr->on_edge(de,a) ) break ;

        // compute distance to cell center and cell width

        cell_dist += cell_width ;         // add half width of prev cell
        cell_width = width_az(de, a, offset) ;
        cell_dist += cell_width ;         // add half width of this cell

        // compute propagation loss contribution of this cell

        const double old_tl = _intensity_az(0) ;

        // Check for an abnormal normalization constant, ie when DE is close to a de branch pt

        if ( de >= max_de ) _new_norm = _norm_az(1,a) ;
        else _new_norm = _norm_az(de,a) ;
        _intensity_az += gaussian(cell_dist, cell_width, _new_norm) ;

        if( _intensity_az(0) / old_tl < THRESHOLD ) break ;
        if( a == 0 ) a += max_az - 1 ;
        else --a ;
    }

    // contribution from higher AZ angles
    // stop when lowest frequency PL changes by < threshold
    // stop if this ray has already contributed, is a duplicate
    // stop at the last ray in the ray fan

    cell_width = initial_width ;
    cell_dist = L - cell_width ;

    a = az + 1 ;
    while( (a % max_az) != az_upper ) {
        if( a == max_az ) a = 0 ;
        if( _duplicate(a,0) ) break ;
        _duplicate(a,0) = true ;
        if( _wave._curr->on_edge(de,a) ) break ;

        // compute distance to cell center and cell width

        cell_dist -= cell_width ;         // remove half width of prev cell
        cell_width = width_az(de, a, offset) ;
        cell_dist -= cell_width ;         // remove half width of this cell

        // compute propagation loss contribution of this cell

        const double old_tl = _intensity_az(0) ;
        // Check for an abnormal normalization constant, ie when DE is close to a de branch pt
        if( de >= max_de ) _new_norm = _norm_az(1,a) ;
        else _new_norm = _norm_az(de,a) ;
        _intensity_az += gaussian(cell_dist, cell_width, _new_norm) ;

        if ( _intensity_az(0) / old_tl < THRESHOLD ) break ;
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

    const size_t max_az = _wave._source_az->size() - 1 ;
    size_t az_wrap ;
    // Check for AZ branch point condition
    if( az+1 >= max_az ) az_wrap = 0 ;
    else az_wrap = az + 1 ;
    const wposition& pos1 = _wave._curr->position ;
    L1 = wvector1(pos1,de,az).distance( wvector1(pos1,de+1,az) ) ;
    if( v < 1e-10 ) {
        length1 = L1 ;
    } else {
        L2 = wvector1(pos1,de,az_wrap).distance( wvector1(pos1,de+1,az_wrap) ) ;
        length1 = (1.0-v) * L1 + v * L2 ;
    }

    // treat nearly zero time offset as a special case

    if( u < 1e-10 ) return 0.5 * length1 ;

    // compute the DE width for the next time step
    //      L1 = cell width from DE to DE+1 along AZ
    //      L2 = cell width from DE to DE+1 along AZ+1
    //      length2 = next distance interpolated across AZ angles
    //      if time offset < zero, use previous instead of next wavefront
    //      treat a nearly zero AZ offset as a special case
    //      create temporary wvector1 variable on the fly

    const wposition& pos2 = (offset(0) < 0.0)
        ? _wave._prev->position
        : _wave._next->position ;
    L1 = wvector1(pos2,de,az).distance( wvector1(pos2,de+1,az) ) ;
    if( v < 1e-10 ) {
        length2 = L1 ;
    } else {
        L2 = wvector1(pos2,de,az_wrap+1).distance( wvector1(pos2,de+1,az_wrap+1) ) ;
        length2 = (1.0-v) * L1 + v * L2 ;
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

    const double u = fabs(offset(0)) / _wave._time_step ;
    const double v = fabs(offset(1)) / (*_wave._source_de).increment(de) ;

    // compute the AZ width for the current time step
    //      L1 = cell width from AZ to AZ+1 along DE
    //      L2 = cell width from AZ to AZ+1 along DE+1
    //      length1 = current distance interpolated across DE angles
    //      treat a nearly zero AZ offset as a special case
    //      create temporary wvector1 variables on the fly

    const wposition& pos1 = _wave._curr->position ;
    const size_t max_az = _wave._source_az->size() - 1 ;
    const size_t max_de = _wave._source_de->size() - 1 ;
    size_t az_wrap ;
    size_t de_upper = de ;
    // Check for DE branch point condition
    if ( de+1 >= max_de ) de_upper = max_de - 2 ;
    else de_upper = de ;
    // Check for AZ branch point condition
    if ( az+1 > max_az ) az_wrap = 0 ;
    else az_wrap = az + 1 ;
    L1 = wvector1(pos1, de, az).distance( wvector1(pos1, de, az_wrap) ) ;
    if( v < 1e-10 || abs(v - 1.0) < 1e-10 ) {
        length1 = L1 ;
    } else {
        L2 = wvector1(pos1, de_upper+1, az).distance( wvector1(pos1, de_upper+1, az_wrap) ) ;
        length1 = (1.0-v) * L1 + v * L2 ;
    }

    // treat nearly zero time offset as a special case

    if( u < 1e-10 ) return 0.5 * length1 ;

    // compute the AZ width for the next time step
    //      L1 = cell width from AZ to AZ+1 along DE
    //      L2 = cell width from AZ to AZ+1 along DE+1
    //      length2 = next distance interpolated across DE angles
    //      if time offset < zero, use previous instead of next wavefront
    //      treat a nearly zero AZ offset as a special case
    //      create temporary wvector1 variables on the fly

    const wposition& pos2 = (offset(0) < 0.0)
        ? _wave._prev->position
        : _wave._next->position ;
    L1 = wvector1(pos2, de, az).distance( wvector1(pos2, de, az_wrap) ) ;
    if( v < 1e-10 || abs(v - 1.0) < 1e-10 ) {
        length2 = L1 ;
    } else {
        L2 = wvector1(pos2, de_upper+1, az).distance( wvector1(pos2, de_upper+1, az_wrap) ) ;
        length2 = (1.0-v) * L1 + v * L2 ;
    }

    // interpolate across times
    return 0.5 * ( (1.0-u) * length1 + u * length2 ) ;
}
