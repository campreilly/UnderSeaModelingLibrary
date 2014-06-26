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
    spreading_model( wave, wave._frequencies->size() ),
    _init_area(wave.num_de(), wave.num_az())
{
    for (unsigned d = 0; d < wave.num_de() - 1; ++d) {
        for (unsigned a = 0; a < wave.num_az() - 1; ++a) {
            double de1 = to_radians(wave.source_de(d));
            double de2 = to_radians(wave.source_de(d + 1));
            double az1 = to_radians(wave.source_az(a));
            double az2 = to_radians(wave.source_az(a + 1));
            _init_area(d, a) = (sin(de2) - sin(de1)) * (az2 - az1);
        }
        _init_area(d, wave.num_az() - 1) = _init_area(d, wave.num_az() - 2);
    }
    for (unsigned a = 0; a < wave._source_az->size() - 1; ++a) {
        _init_area(wave.num_de() - 1, a) = _init_area(wave.num_de() - 2, a);
    }
    _init_area /= _wave._curr->sound_speed(0, 0);
}

/**
 * Estimate intensity as the ratio of current area to initial area.
 */
const vector<double>& spreading_ray::intensity(
    const wposition1& location, unsigned de, unsigned az,
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
    const double loss = _init_area(de, az) * sound_speed(0, 0) / area;
    for (unsigned f = 0; f < _wave._frequencies->size(); ++f) {
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
 * Find the distance between de-1/de and de/de+1, then add half of each to
 * find the distance centered at de.
 */
double width_de( unsigned de, unsigned az, const vector<double>& offset ) {

}

/**
 * Find the distance between az-1/az and az/az+1, then add half of each to
 * find the distance centered at az.
 */
double width_az( unsigned de, unsigned az, const vector<double>& offset ) {

}
