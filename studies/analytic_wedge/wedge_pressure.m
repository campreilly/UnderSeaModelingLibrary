%%
% Wave number integration for a specific image source in the 
% Deane/Buckingham wedge solution.  
% Based on equation (23) in the paper.
%
%   A = wedge_coeff( ...
%     wedge_theta, density, speed, atten, speed_shear, atten_shear )
%
%   P           = complex pressure for one image source
%   tolerance   = relative size for cutting off Bessel series
%   wave_number = magnitude of the acoustic wave number
%   bessel_order= order of the Bessel function being evaluated
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
% fixes the values of num_surface, num_bottom, and target_phi.
% For each wave_theta in the wavenumber integration, this coefficient
% must be evaluated for multiple values of bessel_order.
%
function P = wedge_pressure( ...
    tolerance, wave_number, num_surface, num_bottom, ...
    target_range, target_theta, target_phi, ...
    wedge_theta, density, speed, atten, speed_shear, atten_shear )

    sin_target_theta = sin(target_theta) ;
    cos_target_theta = cos(target_theta) ;

    P = 1i * wave_number / pi * quad(@wave_number_integ,0,pi/2) ;
    
    %%
    % Integrand of wave number contributions to pressure
    %
    function integ = wave_number_integ( theta )
        fprintf('wave_number_integ(%f)\n',theta) ;
        sin_theta = sin( theta ) ;
        cos_theta = cos( theta ) ;
        omega = wave_number .* target_range .* sin_theta .* sin_target_theta ;
        omegaZ = wave_number .* target_range .* cos_theta .* cos_target_theta ;
            
        % contribution from first term in the Bessel series
            
        a = wedge_coeff( ...
            theta, 0, num_surface, num_bottom, target_phi, ...
            wedge_theta, density, speed, atten, speed_shear, atten_shear ) ;
        integ = 0.5 * a * bessel(0,omega) ;
        
        % contribution from other term in the Bessel series
        for v = 0:10
            fprintf('\tv=%d\n',v);
            % odd numbered orders
            bessel_order = 2*v+1 ;
            a = wedge_coeff( ...
                theta, bessel_order, num_surface, num_bottom, target_phi, ...
                wedge_theta, density, speed, atten, speed_shear, atten_shear ) ;
            new = 1i * (-1)^v * a * bessel(bessel_order,omega) ;
            
            % even numbered orders
            if ( v > 0 )
                bessel_order = bessel_order-1 ;
                a = wedge_coeff( ...
                    theta, bessel_order, num_surface, num_bottom, target_phi, ...
                    wedge_theta, density, speed, atten, speed_shear, atten_shear ) ;
                new = new + (-1)^v * a * bessel(bessel_order,omega) ;
            end
            
            % incorporate new contribution into the sum
            % exit early if the new contribution is small
            integ = integ + new ;
            if ( max( abs(new ./integ)) < tolerance )
                break ;
            end
        end
        integ = exp( 1i*omegaZ ) .* sin_theta * integ ;
    end


end