/*
 * @file eigenverb_collection.cc
 */

#include <usml/waveq3d/eigenverb_collection.h>

/*
 * Call for tabulating upper collisions.
 */
eigevern_collection::notifyUpperCollision( size_t de, size_t az,
        double dt, double grazing, double speed,
        const wposition1& position, const wvector1& ndirection,
        const wave_queue& wave, size_t ID )
{
    eigenverb verb ;
    create_eigenverb( de, az, dt, grazing, speed, position, ndirection, wave, verb ) ;
     // Don't bother adding the ray it its too quiet
    if ( verb.intensity(0) > 1e-10 ) {
        if( ID == _source_origin ) {
            _surface.push_back( verb ) ;
        } else {
            std::list<eigenverb_list>::iterator it = _upper.begin() ;
            (*it+ID).push_back( verb ) ;
        }
    }
}

/**
 * Creates an eigenverb from the provided data
 */
void eigenverb_model::create_eigenverb( size_t de, size_t az,
               double dt, double grazing, double speed,
               const wposition1& position, const wvector1& ndirection,
               const wave_queue& wave, eigenverb& verb ) const
{
    verb.de_index = de ;
    verb.az_index = az ;
    verb.launch_az = wave.source_az(az) ;
    verb.launch_de = wave.source_de(de) ;
    verb.distance = wave.curr()->path_length(de,az) + speed * dt ;
    verb.travel_time = wave.time() + dt ;
    verb.grazing = grazing ;
    verb.sound_speed = speed ;
    verb.position = position ;
    verb.direction = ndirection ;
    verb.frequencies = wave.frequencies() ;
    verb.surface = wave.curr()->surface(de,az) ;
    verb.bottom = wave.curr()->bottom(de,az) ;

        // Calculate the one way TL and the width of the gaussian
        // at the time of impact with the boundary.
    double true_distance = verb.distance ;
    double spreading_loss = 1.0 / (true_distance * true_distance) ;
    vector<double> amp( wave.frequencies()->size(), spreading_loss ) ;
    vector<double> boundary_loss = pow( 10.0, -0.1 * wave.curr()->attenuation(de,az) ) ;
    verb.intensity = element_prod( amp, boundary_loss ) ;

    double delta_de ;
    if( de == 0 ) {
        delta_de = M_PI * ( wave.source_de(de+1) - wave.source_de(de) ) / 180.0  ;
    } else {
        delta_de = M_PI * ( wave.source_de(de+1) - wave.source_de(de-1) ) / 360.0  ;
    }
    verb.sigma_de = true_distance * delta_de / sin(grazing) ;
    double delta_az = M_PI * ( wave.source_az(az+1) - wave.source_az(az) ) / 180.0 ;
    verb.sigma_az = delta_az * cos(grazing) * true_distance ;   // horizontal distance * azimuthal spacing
    if( abs(grazing) > (M_PI_2 - 1e-10) ) verb.sigma_az = TWO_PI * true_distance ;

    #ifdef EIGENVERB_COLLISION_DEBUG
        cout << "\t---Added eigenverb to collection---" << endl ;
        cout << "\tverb de: " << verb.launch_de << " az: " << verb.launch_az
             << " time: " << verb.travel_time << endl ;
        cout << "\tgrazing: " << verb.grazing*180.0/M_PI << " speed: " << verb.sound_speed << endl ;
        cout << "\tintensity: " << verb.intensity << " sigma_de: " << verb.sigma_de
             << " sigma_az: " << verb.sigma_az << endl ;
    #endif
}
