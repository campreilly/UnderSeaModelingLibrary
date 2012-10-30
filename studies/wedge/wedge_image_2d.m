%%
% Wave number spectrum for a specific image source in the 
% Deane/Buckingham 2-D wedge solution.  
% Based on equation (28) in the paper.
%
%   H = wedge_image_2d( ...
%     wave_number, wave_theta, num_surface, num_bottom, ...
%     target_rho, target_theta, ...
%     wedge_theta, density, speed, atten, speed_shear, atten_shear )
%
%   H           = complex pressure spectrum for one image source
%   wave_number = magnitude of the acoustic wave number
%   wave_theta  = magnitude of the acoustic wave number
%   num_surface = number of surface bounces for current source
%   num_bottom  = number of bottom bounces for current source
%   target_rho= target distance from source
%   target_theta= target DE relative to source
%   target_phi  = target AZ relative to source
%   wedge_theta = DE angle of the bottom slope
%   density     = Ratio of bottom density to water density 
%   speed       = Ratio of compressional sound speed in the bottom to
%               the sound speed in water. 
%   atten       = Compressional wave attenuation in bottom (dB/wavelength).  
%               No compressional attenuation if this is zero.
%   speed_shear = Ratio of shear speed in the bottom to
%               the compressional sound speed in water. 
%               No shear component if this is zero.
%   atten_shear = Shear wave attenuation in bottom (dB/wavelength).  
%               No shear attenuation if this is zero.
%
% The environmental conditions fix the values of wedge_theta, density, 
% speed, atten, speed_shear, and atten_shear.  The source/target geometry
% fixes the values of num_surface, num_bottom, target_rho, target_theta, 
% and target_phi. This routine uses these to generate a Bessel series 
% solution for each wave_theta in the sprectrum.
%
function H = wedge_image_2d( ...
    wave_number, wave_theta, num_surface, num_bottom, ...
    target_rho, target_theta, ...
    wedge_theta, density, speed, atten, speed_shear, atten_shear )

    % pre-compute some frequently used numbers
    
    sin_target_theta = sin( target_theta ) ;
    cos_target_theta = cos( target_theta ) ;

    sin_theta = sin( wave_theta ) ;
    cos_theta = cos( wave_theta ) ;
    
    omega = wave_number .* target_rho .* sin_theta .* sin_target_theta ;
    omegaZ = wave_number .* target_rho .* cos_theta .* cos_target_theta ;

    % compute contribution from first term in the Bessel series

    loss = 1.0 ;
    for b = 1:num_bottom
        theta = 2 .* wedge_theta .* (b-1) ;
        angle = acos( sin_theta .* sin(theta) + cos_theta .* cos(theta) ) ;
        loss = loss * reflection( angle, ...
            density, speed, atten, speed_shear, atten_shear ) ;
    end
    H = (1i * wave_number) .* exp( 1i*omegaZ ) .* (-1)^num_surface ...
      .* loss .* besselj(0,omega) .* sin_theta ;
    
end