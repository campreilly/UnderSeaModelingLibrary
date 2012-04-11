%%
% Compute the Rayleigh reflection loss model for bottom sediments.
%
% syntax:   loss = reflect_loss_rayleigh( angles_water, density_bottom, 
%                  speed_bottom, att_bottom, speed_shear, att_shear )
%
%           angles = grazing angle in water (deg)
%           density_bottom = sediment material density (kg/m^3)
%           speed_bottom = compressional sound speed in bottom (m/s)
%           att_bottom = compressional attenuation (dB/wavelength)
%           speed_shear = shear sound speed in bottom (m/s)
%           att_shear = shear attenuation (dB/wavelength)
%           loss = reflection loss (dB)
%
function loss = reflect_loss_rayleigh( angles_water, density_bottom, speed_bottom, att_bottom, speed_shear, att_shear )

density_water = 1000.0 ;    % kg/m^3
speed_water = 1500.0 ;      % m/s

speed_bottom = complex_speed( speed_bottom, att_bottom ) ;
speed_shear = complex_speed( speed_shear, att_shear ) ;

angles_bottom = acos( cos(angles_water) / speed_water * speed_bottom ) ;
angles_shear  = acos( cos(angles_water) / speed_water * speed_shear ) ;

impedance_water  = impedance( angles_water, density_water, speed_water ) ;
impedance_bottom = impedance( angles_bottom, density_bottom, speed_bottom ) ;
impedance_shear  = impedance( angles_shear, density_bottom, speed_shear ) ;

impedance_bottom = impedance_bottom .* cos(2*angles_shear).^2 ...
                 + impedance_shear .* sin(2*angles_shear).^2 ;

loss = ( impedance_bottom - impedance_water ) ...
    ./ ( impedance_bottom + impedance_water ) ;

   
loss = -20.0 * log10( abs(loss) ) ;

end

%%
% Compute the complex speed as a mixture of speed and attenuation
% Using formula 1.43 and 1.46 from Jensen, Kuperman, et. al. 
% Computational Acoustics, 2nd Edition.
%
% syntax:   c = complex_speed( speed, attenuation, frequency )
%
%           speed = speed of sound (m/s)
%           attenuation = attenuation (dB/wavelength)
%           c = complex speed of sound (m/s)
%
function c = complex_speed( speed, attenuation )
    convert = (20.0*log10(exp(1))) * (2*pi) ;
    c = speed * ( 1 + i * attenuation / convert ) ;
end

%%
% Compute the interface impedance 
% Using formula 1.43 and 1.46 from Jensen, Kuperman, et. al. 
% Computational Acoustics, 2nd Edition.
%
% syntax:   Z = impedance( angles, density, speed ) 
%
%           angles = complex grazing angles (deg)
%           density = material density (kg/m^3)
%           speed = complex speed of sound, including attenuation (m/s)
%           Z = plane wave acoustic impedance
%
function Z = impedance( angles, density, speed ) 
    Z = density * speed ./ sin(angles) ;
end