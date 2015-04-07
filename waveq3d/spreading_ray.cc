/**
 * @file spreading_ray.cc
 * Spreading loss based on classic ray theory.
 */
#include <usml/waveq3d/spreading_ray.h>

using namespace usml::waveq3d;

/**
 * Estimate initial ensonfied area between rays at radius of 1 meter.
 */
spreading_ray::spreading_ray(wave_queue& wave) :
    spreading_model( wave, wave._frequencies->size() )
{
    for (size_t d = 0; d < wave.num_de() - 1; ++d) {
        for (size_t a = 0; a < wave.num_az() - 1; ++a) {
            double de1 = to_radians(wave.source_de(d));
            double de2 = to_radians(wave.source_de(d + 1));
            double az1 = to_radians(wave.source_az(a));
            double az2 = to_radians(wave.source_az(a + 1));
            _init_area(d, a) = (sin(de2) - sin(de1)) * (az2 - az1);
        }
        _init_area(d, wave.num_az() - 1) = _init_area(d, wave.num_az() - 2);
    }
    for (size_t a = 0; a < wave._source_az->size() - 1; ++a) {
        _init_area(wave.num_de() - 1, a) = _init_area(wave.num_de() - 2, a);
    }
    _init_sound_speed = _wave._curr->sound_speed(0, 0);
}

/**
 * Estimate intensity as the ratio of current area to initial area.
 */
const vector<double>& spreading_ray::intensity(
    const wposition1& location, size_t de, size_t az,
    const vector<double>& offset, const vector<double>& distance )
{
    // which box has target in it?

    if (offset(1) < 0.0) --de;
    if (offset(2) < 0.0) --az;

    // get sound speed at target

    matrix<double> sound_speed(1, 1);
    wposition loc(1, 1);
    loc.rho(0, 0, location.rho());
    loc.theta(0, 0, location.theta());
    loc.phi(0, 0, location.phi());
    _wave._ocean.profile().sound_speed(loc, &sound_speed);

    // compare area of this box to original area
    // linear interpolation between two wavefronts

//    cout << "*** time=" << _wave._time
//         << " de(" << de << ")=" << (*_wave._source_de)(de)
//         << " de(" << de+1 << ")=" << (*_wave._source_de)(de+1)
//         << " az(" << az << ")=" << (*_wave._source_az)(az)
//         << " az(" << az+1 << ")=" << (*_wave._source_az)(az+1)
//         << " srf=" << (*_wave._curr).surface(de,az)
//         << " btm=" << (*_wave._curr).bottom(de,az)
//         << " cst=" << (*_wave._curr).caustic(de,az)
//         << endl
//         << " offset(time)=" << offset(0)
//         << " offset(de)=" << offset(1)
//         << " offset(az)=" << offset(2)
//         << endl ;

    const wposition& pos1 = _wave._curr->position;
    const wvector1 t1p1(pos1, de, az);
    const wvector1 t1p2(pos1, de + 1, az);
    const wvector1 t1p3(pos1, de + 1, az + 1);
    const wvector1 t1p4(pos1, de, az + 1);
    const double area1 = t1p1.area(t1p2, t1p3, t1p4);

    double area2 = 0.0;
    if (offset(0) < 0.0) {
        const wposition& pos2 = _wave._prev->position;
        const wvector1 t2p1(pos2, de, az);
        const wvector1 t2p2(pos2, de + 1, az);
        const wvector1 t2p3(pos2, de + 1, az + 1);
        const wvector1 t2p4(pos2, de, az + 1);
        area2 = t2p1.area(t2p2, t2p3, t2p4);
    } else {
        const wposition& pos2 = _wave._next->position;
        const wvector1 t2p1(pos2, de, az);
        const wvector1 t2p2(pos2, de + 1, az);
        const wvector1 t2p3(pos2, de + 1, az + 1);
        const wvector1 t2p4(pos2, de, az + 1);
        area2 = t2p1.area(t2p2, t2p3, t2p4);
    }

    double u = fabs(offset(0)) / _wave._time_step;
    const double area = (1.0 - u) * area1 + u * area2;
//    cout << " area1=" << area1 << " area2=" << area2
//         << " u=" << u << " area=" << area << endl ;
    const double loss = _init_area(de, az) * sound_speed(0, 0) /
    		( area * _init_sound_speed ) ;
    for (size_t f = 0; f < _wave._frequencies->size(); ++f) {
        _spread(f) = loss ;
    }

    // attenuate signals outside of the ray fan
    // keep a constant level for 3 extra beam widths

    u = fabs(offset(1)) / _wave._source_de->increment(de) - 1.0 ;
    if ( u > 4.0 ) _spread *= 0.0 ;

    u = fabs(offset(2)) / _wave._source_az->increment(az) - 1.0 ;
    if ( u > 4.0 ) _spread *= 0.0 ;

    return _spread ;
}

/**
 * Compute the harmonic mean of the distance between the position
 * on the wavefront that is DE+1 and DE-1 from DE. Divides the result
 * by 1/2, as the width of the gaussian cell is half of the distance.
 */
double spreading_ray::width_de(
        size_t de, size_t az, const vector<double>& offset )
{
    // Prevent the algorithm from running into problems with the
    // DE is at the edge of the index array.
    const size_t size_de = _wave._source_de->size() - 1 ;
    size_t de_max, de_min, de_center ;
    if ( de >= size_de ) {
        de_max = size_de ;
        de_center = size_de - 1 ;
        de_min = size_de - 2 ;
    } else if ( de == 0 ) {
        de_max = de + 2 ;
        de_center = 1 ;
        de_min = 0 ;
    } else {
        de_max = de + 1 ;
        de_center = de ;
        de_min = de - 1 ;
    }

    // Find the distances from these points on the wavefront to
    // each other
    const wposition& pos1 = _wave._curr->position ;
    const wvector1 A(pos1, de_min, az) ;
    const wvector1 B(pos1, de_center, az) ;
    const wvector1 C(pos1, de_max, az) ;
    double width1 = A.distance(B) ;
    double width2 = B.distance(C) ;

    // Compute half the harmonic mean
    double u =  1.0 / width1 + 1.0 / width2 ;
    u = 1.0 / u ;
    return u ;
}

/**
 * Compute the harmonic mean of the distance between the position
 * on the wavefront that is AZ+1 and AZ-1 from AZ. Divides the result
 * by 1/2, as the width of the gaussian cell is half of the distance.
 */
double spreading_ray::width_az(
        size_t de, size_t az, const vector<double>& offset )
{
    // Prevent the algorithm from running into problems with the
    // AZ is at the edge of the index array.
    const size_t size = _wave._source_az->size() - 1 ;
    size_t az_min, az_max ;
    if ( az >= size ) {
        az_max = 0 ;
        az_min = az - 1 ;
    } else if ( az == 0 ) {
        az_max = az + 1 ;
        az_min = size ;
    } else {
        az_max = az + 1 ;
        az_min = az - 1 ;
    }

    // Find the distances from these points on the wavefront to
    // each other
    const wposition& pos1 = _wave._curr->position ;
    const wvector1 A(pos1, de, az_min) ;
    const wvector1 B(pos1, de, az) ;
    const wvector1 C(pos1, de, az_max) ;
    double width1 = A.distance(B) ;
    double width2 = B.distance(C) ;

    // Compute half the harmonic mean
    double u =  1.0 / width1 + 1.0 / width2 ;
    u = 1.0 / u ;
    return u ;
}
