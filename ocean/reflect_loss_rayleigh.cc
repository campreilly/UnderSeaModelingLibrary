/**
 * @file reflect_loss_rayleigh.cc
 * Models plane wave reflection from a flat fluid-solid interface.
 */
#include <usml/ocean/reflect_loss_rayleigh.h>

using namespace usml::ocean ;

/**
 * Converts attenuation in dB/wavelength units into the loss tangent
 * needed for the complex sound speed.  This is the same as the
 * (1.0/54.5751) factor defined in  * F.B. Jensen, W.A. Kuperman,
 * M.B. Porter, H. Schmidt, "Computational Ocean Acoustics",
 * equation 1.46.
 *
 */
static const double ATT_CONVERT = 1.0 / (20.0*M_LOG10E*TWO_PI);

/**
 * Reflection loss parameter lookup from table 1.3 in
 * F.B. Jensen, W.A. Kuperman, M.B. Porter, H. Schmidt,
 * "Computational Ocean Acoustics", pp. 41.
 * Does not implement the weak, depth dependent, shear
 * in slit, sand, and gravel.  Don't use this table if
 * more precision is called for.
 */
struct reflect_loss_rayleigh::bottom_type_table
    reflect_loss_rayleigh::lookup[] =
{
//    type        den  spd   att  shear  satt
//    ---------   ---  ----  ---  -----  ----
    { CLAY,       1.5, 1.00, 0.2, 0.00,  1.0  },
    { SILT,       1.7, 1.05, 1.0, 0.00,  1.5  },
    { SAND,       1.9, 1.10, 0.8, 0.00,  2.5  },
    { GRAVEL,     2.0, 1.20, 0.6, 0.00,  1.5  },
    { MORAINE,    2.1, 1.30, 0.4, 0.40,  1.0  },
    { CHALK,      2.2, 1.60, 0.2, 0.67,  0.5  },
    { LIMESTONE,  2.4, 2.00, 0.1, 1.00,  0.2  },
    { BASALT,     2.7, 3.50, 0.1, 1.67,  0.2  },
} ;

/**
 * Initialize model with impedance mis-match factors.
 */
reflect_loss_rayleigh::reflect_loss_rayleigh(bottom_type_enum type)
    :
    _density_water(1000.0),
    _speed_water(1500.0),
    _density_bottom( _density_water * lookup[(int)type].density ),
    _speed_bottom( _speed_water * lookup[(int)type].speed ),
    _att_bottom( lookup[(int)type].att_bottom * ATT_CONVERT ),
    _speed_shear( _speed_water * lookup[(int)type].speed_shear ),
    _att_shear( lookup[(int)type].att_shear * ATT_CONVERT )
{
}

/**
 * Initialize model with impedance mis-match factors.
 */
reflect_loss_rayleigh::reflect_loss_rayleigh(int type)
    :
    _density_water(1000.0),
    _speed_water(1500.0),
    _density_bottom( _density_water * lookup[type].density ),
    _speed_bottom( _speed_water * lookup[type].speed ),
    _att_bottom( lookup[type].att_bottom * ATT_CONVERT ),
    _speed_shear( _speed_water * lookup[type].speed_shear ),
    _att_shear( lookup[type].att_shear * ATT_CONVERT )
{
}

/**
 * Initialize model with impedance mis-match factors.
 */
reflect_loss_rayleigh::reflect_loss_rayleigh(
    double density, double speed, double att_bottom,
    double speed_shear, double att_shear
) :
    _density_water(1000.0),
    _speed_water(1500.0),
    _density_bottom( _density_water * density ),
    _speed_bottom( _speed_water * speed ),
    _att_bottom( att_bottom * ATT_CONVERT ),
    _speed_shear( _speed_water * speed_shear ),
    _att_shear( att_shear * ATT_CONVERT )
{
}

/**
 * Computes the broadband reflection loss and phase change.
 */
void reflect_loss_rayleigh::reflect_loss(
    const wposition1& location,
    const seq_vector& frequencies, double angle,
    vector<double>* amplitude, vector<double>* phase )
{
    if ( angle >= M_PI_2 ) angle = M_PI_2 - 1e-10 ;

    angle = M_PI_2 - angle ;

    // compute acoustic impedance in water

    double Zw = cos(angle) / (_speed_water * _density_water) ;

    // compute compression wave reflection components

    complex<double> cosAp, cosAs ;
    complex<double> Zb = impedance(
        _density_bottom, _speed_bottom, _att_bottom, angle, &cosAp, false ) ;

    // compute shear wave reflection components

    if ( _speed_shear != 0.0 || _att_shear != 0.0 ) {
        const complex<double> Zs = impedance(
            _density_bottom, _speed_shear, _att_shear, angle, &cosAs, true ) ;
        const complex<double> sinAs = sqrt( 1.0 - cosAs*cosAs ) ;
        const complex<double> cos2As = 2.0 * cosAs * cosAs - 1.0 ;
        const complex<double> sin2As = 2.0 * sinAs * cosAs ;
        Zb =  1.0 / ((1.0/Zb) * cos2As * cos2As + Zs * sin2As * sin2As ) ;
    }

    // compute complex reflection coefficient

    complex<double> R = ( Zb - Zw ) / ( Zb + Zw ) ;
    noalias(*amplitude) = -20.0 * log10( abs(R) ) *
        scalar_vector<double>( frequencies.size(), 1.0 ) ;
    if ( phase ) {
        noalias(*phase) = arg(R) *
            scalar_vector<double>( frequencies.size(), 1.0 ) ;
    }
}

/**
 * Compute impedance for compression or shear waves with attenuation.
 */
complex<double> reflect_loss_rayleigh::impedance(
    double density, double speed, double attenuation, double angle,
    complex< double >* cosA, bool shear )
{
    const complex< double > c( speed, -attenuation*speed ) ;
    complex< double > sinA = sin(angle) * c / _speed_water ;
    *cosA = sqrt( 1.0 - sinA*sinA ) ;
    complex<double> value ;
    if( shear ) {
        value = c * density / *cosA ;
    } else {
        value = *cosA / (c * density) ;
    }
    return value ;
}
