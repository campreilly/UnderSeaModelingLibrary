%%
% Wave number spectrum for a specific image source/target combination in
% equation 28 of the Deane/Buckingham March 1993 paper.  Limits the
% evaluation to 2-D contributions, those in the plane of the source.
%
%   spectrum = wedge_spectrum_2d( num_surface, num_bottom, ...
%       wave_number, theta, range, zeta )
%
%   spectrum    = complex pressure spectrum for one image source
%   num_surface = number of surface bounces for current source
%   num_bottom  = number of bottom bounces for current source
%   range       = range from source for current target
%   zeta        = depression/elevation angle for current target relative to source
%   wave_number = magnitude of the acoustic wave number (scalar)
%   theta       = wave number depression/elevation angle (vector)
%
% Relies on global variables for the bottom properties of wedge_angle,
% density, speed, atten, speed_shear, atten_shear.
%
function spectrum = wedge_spectrum_2d( num_surface, num_bottom, ...
    wave_number, theta, range, zeta)

    global wedge_angle density speed atten speed_shear atten_shear

    % pre-compute some frequently used numbers

    sin_zeta = sin( zeta ) ;
    cos_zeta = cos( zeta ) ;

    sin_theta = sin( theta ) ;
    cos_theta = cos( theta ) ;

    omega = wave_number .* range .* sin_theta .* sin_zeta ;
    omegaZ = wave_number .* range .* cos_theta .* cos_zeta ;

    % compute the product of all interface loss interactions

    loss = -1^num_surface ;
    for b = 1:num_bottom
        big_theta = 2 .* wedge_angle .* (b-1) ;
        normal_angle = acos( sin_theta .* sin(big_theta) + cos_theta .* cos(big_theta) ) ;
        loss = loss .* reflection( normal_angle, ...
            density, speed, atten, speed_shear, atten_shear ) ;
    end

    % compute contribution from first term in the Bessel series

    spectrum = (1i * wave_number) .* exp( -1i*omegaZ ) ...
        .* loss .* besselj(0,omega) .* sin_theta ;
end
