%%
% Wave number spectrum for multiple source images in the 
% Deane/Buckingham 2-D wedge solution. 
% Based on equations (24) and (25) in the paper.
% 
%   H = wedge_spectrum_2d( ...
%     wave_number, wave_theta, max_bottom, tolerance, ...
%     source_rho, source_theta, target_rho, target_theta, ...
%     wedge_theta, density, speed, atten, speed_shear, atten_shear )
%
%   H           = complex pressure spectrum from multiple source images
%   wave_number = magnitude of the acoustic wave number
%   wave_theta  = magnitude of the acoustic wave number
%   max_bottom  = max number of bottom bounces to model
%   tolerance   = relative size for cutting off # of bottom bounces early
%   source_rho  = source range from apex
%   source_theta= source DE relative to apex
%   target_rho  = target range from apex
%   target_theta= target DE relative to apex
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
% fixes the values of source_rho, source_theta, target_rho, and
% target_theta. This routine uses these to generate a spectrum from
% multiple source images as a function of wave_theta.  Although
% the maximum number of bottom bounces can be controlled using max_bottom,
% the series terminates prematurely if the current contribution
% is smaller than tolerance.
%
function H = wedge_spectrum_2d( ...
    wave_number, wave_theta, max_bottom, tolerance, ...
    source_rho, source_theta, target_rho, target_theta, ...
    wedge_theta, density, speed, atten, speed_shear, atten_shear )

    target_phi = 0.0 ;
    
    % source contribution = s(0,0) term
    
    [ rho, theta ] = spherical_add( ...
        target_rho, target_theta, target_phi, ...
        -source_rho, source_theta, 0.0 ) ;
%     H = wedge_image_2d( ...
%         wave_number, wave_theta, 0, 0, rho, theta, ...
%         wedge_theta, density, speed, atten, speed_shear, atten_shear ) ; 
    H = exp( 1i * spherical_dot( rho, theta, 0, wave_number, wave_theta, 0.0 ) ...
        ./ ( wave_number .* cos(wave_theta) ) ) ;
    
    % 1st surface image = s(1,0) term
    
%     [ rho, theta ] = spherical_add( ...
%         target_rho, target_theta, target_phi, ...
%         -source_rho, pi-source_theta, 0.0 ) ;
%     H = H + wedge_image_2d( ...
%         wave_number, wave_theta, 1, 0, rho, theta, ...
%         wedge_theta, density, speed, atten, speed_shear, atten_shear ) ;

    % loop through images that include bottom reflection
    
    for num_bottom = 1:max_bottom
        
        % source->surface->bottom->target path = s'(n,n) term
        
        [ rho, theta ] = spherical_add( ...
            target_rho, target_theta, target_phi, ...
            -source_rho, -source_theta-2*wedge_theta*(num_bottom-1), 0.0 ) ;
        new = wedge_image_2d( ...
            wave_number, wave_theta, num_bottom, num_bottom, rho, theta, ...
            wedge_theta, density, speed, atten, speed_shear, atten_shear ) ;
        
        % source->bottom->target path = s'(n-1,n) term
        
        [ rho, theta ] = spherical_add( ...
            target_rho, target_theta, target_phi, ...
            -source_rho, source_theta-2*wedge_theta*num_bottom, 0.0 ) ;
        new = new + wedge_image_2d( ...
            wave_number, wave_theta, num_bottom-1, num_bottom, rho, theta, ...
            wedge_theta, density, speed, atten, speed_shear, atten_shear ) ;
        
        % source->bottom->surface->target path = s(n,n) term
        
        [ rho, theta ] = spherical_add( ...
            target_rho, target_theta, target_phi, ...
            -source_rho, -source_theta+2*wedge_theta*num_bottom, 0.0 ) ;
        new = new + wedge_image_2d( ...
            wave_number, wave_theta, num_bottom, num_bottom, rho, theta, ...
            wedge_theta, density, speed, atten, speed_shear, atten_shear ) ;
        
        % source->surface->bottom->surface->target path = s(n+1,n) term
        
        [ rho, theta ] = spherical_add( ...
            target_rho, target_theta, target_phi, ...
            -source_rho, source_theta+2*wedge_theta*(num_bottom+1), 0.0 ) ;
        new = new + wedge_image_2d( ...
            wave_number, wave_theta, num_bottom+1, num_bottom, rho, theta, ...
            wedge_theta, density, speed, atten, speed_shear, atten_shear ) ;
        
        % incorporate new contribution into the sum
        % exit early if the new contribution is small
        
        H = H + new ;
        if ( max( abs(new./H)) < tolerance )
            break ;
        end
        
    end    
    
end