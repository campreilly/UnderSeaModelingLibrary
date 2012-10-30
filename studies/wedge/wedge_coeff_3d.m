%%
% Compute coefficent for a specific term in the Bessel expansion
% of the Deane/Buckingham 3-D wedge solution.  
% Based on equation (26) in the paper.
%
%   A = wedge_coeff_3d( ...
%     wave_theta, bessel_order, num_surface, num_bottom, target_phi, ...
%     wedge_theta, density, speed, atten, speed_shear, atten_shear )
%
%   A           = coefficient for this term in Bessel expansion.
%   wave_theta  = wave number DE relative to source
%   bessel_order= order of the Bessel function being evaluated
%   num_surface = number of surface bounces for current source
%   num_bottom  = number of bottom bounces for current source
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
function A = wedge_coeff_3d( ...
    wave_theta, bessel_order, num_surface, num_bottom, target_phi, ...
    wedge_theta, density, speed, atten, speed_shear, atten_shear )

    sin_theta = sin(wave_theta) ;
    cos_theta = cos(wave_theta) ;

    A = (-1)^num_surface * 2 * cos( bessel_order * target_phi ) ...
      * quad(@reflection_integ,0,pi) ;
    
    %%
    % Integrand of bottom losses for a given source.
    %
    function integ = reflection_integ( phi )
        integ = reflection_loss( phi ) * cos( bessel_order * phi ) ;
    end

    %%
    % Product of bottom losses for a given source.
    %
    function loss = reflection_loss( phi )
        loss = 1.0 ;
        for b = 1:num_bottom
            loss = loss * reflection( reflection_angle( b, phi ), ...
                density, speed, atten, speed_shear, atten_shear ) ;
        end
    end

    %%
    % Computes reflection angle relative to the normal.
    %
    function angle = reflection_angle( b, phi )
        theta = 2 .* wedge_theta .* (b-1) ;
        angle = acos( sin_theta .* sin(theta) .* cos(phi) + cos_theta .* cos(theta) ) ;
    end

end