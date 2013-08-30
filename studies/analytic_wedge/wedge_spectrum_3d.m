%%
% Wave number spectrum for a specific image source in the 
% Deane/Buckingham 3-D wedge solution.  Based on equation (23) in the paper.
%
%   H = wedge_spectrum_3d( ...
%     wave_number, wave_theta, tolerance, num_surface, num_bottom, ...
%     target_range, target_theta, target_phi, ...
%     wedge_theta, density, speed, atten, speed_shear, atten_shear )
%
%   H           = complex pressure spectrum for one image source
%   wave_number = magnitude of the acoustic wave number
%   wave_theta  = magnitude of the acoustic wave number
%   tolerance   = relative size for cutting off Bessel series
%   num_surface = number of surface bounces for current source
%   num_bottom  = number of bottom bounces for current source
%   target_range= target distance from source
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
% fixes the values of num_surface, num_bottom, target_range, target_theta, 
% and target_phi. This routine uses these to generate a Bessel series 
% solution for each wave_theta in the sprectrum.
%
function H = wedge_spectrum_3d( ...
    wave_number, wave_theta, tolerance, num_surface, num_bottom, ...
    target_range, target_theta, target_phi, ...
    wedge_theta, density, speed, atten, speed_shear, atten_shear )

    % pre-compute some frequently used numbers
    
    sin_target_theta = sin( target_theta ) ;
    cos_target_theta = cos( target_theta ) ;

    sin_theta = sin( wave_theta ) ;
    cos_theta = cos( wave_theta ) ;
    
    omega = wave_number .* target_range .* sin_theta .* sin_target_theta ;
    omegaZ = wave_number .* target_range .* cos_theta .* cos_target_theta ;

    % compute contribution from first term in the Bessel series

    a = wedge_coeff_3d( ...
        theta, 0, num_surface, num_bottom, target_phi, ...
        wedge_theta, density, speed, atten, speed_shear, atten_shear ) ;
    H = 0.5 .* a .* bessel(0,omega) ;

    % compute contributions from other term in the Bessel series
    % not required for targets in same plane as source
    
    if ( abs(sin(target_phi)) > 1e-10 )
        for v = 0:10
            fprintf('wedge_spectrum: v=%d\n',v);

            % odd numbered terms in the Bessel series
            bessel_order = 2*v+1 ;
            a = wedge_coeff( ...
                theta, bessel_order, num_surface, num_bottom, target_phi, ...
                wedge_theta, density, speed, atten, speed_shear, atten_shear ) ;
            new = 1i .* (-1)^v .* a .* bessel(bessel_order,omega) ;

            % even numbered terms in the Bessel series
            if ( v > 0 )
                bessel_order = bessel_order-1 ;
                a = wedge_coeff( ...
                    theta, bessel_order, num_surface, num_bottom, target_phi, ...
                    wedge_theta, density, speed, atten, speed_shear, atten_shear ) ;
                new = new + (-1)^v .* a .* bessel(bessel_order,omega) ;
            end
            H = H + new ;

            % terminate the Bessel series if contribution is too small
            if ( norm( new ./ H ) < tolerance )
                break ;
            end
        end
    end
    
    H = (1i * wave_number / pi) .* exp( 1i*omegaZ ) .* sin_theta .* H ;
end