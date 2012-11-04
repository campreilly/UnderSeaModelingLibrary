%%
% Computes complex pressure in the Deane/Buckingham wedge solution for a
% specific combination of source and target locations.  Limits the
% evaluation to 2-D contributions, those in the plane of the source. Sums
% the wedge_integ_2d() contributions over all combinations of source and
% bottom reflection.  Implements equation (24) in March 1993 paper.
% 
%   pressure = wedge_pressure_2d( ...
%       wave_number, max_bottom, tolerance, ...
%       source_range, source_zeta, target_range, target_zeta )
%
%   pressure        = complex pressure from multiple source images
%   wave_number     = magnitude of the acoustic wave number
%   max_bottom      = max number of bottom bounces to model
%   tolerance       = relative size for cutting off # of bottom bounces early
%   source_range    = source range from apex
%   source_zeta     = source D/E angle relative to ocean bottom
%   target_range    = target range from apex
%   target_zeta     = target D/E angle relative to ocean bottom
%
% Although the maximum number of bottom bounces can be controlled using
% max_bottom, the series terminates prematurely if the current contribution
% is smaller than tolerance.
%
% Relies on global variable for the bottom properties of wedge_angle,
% density, speed, atten, speed_shear, atten_shear.
%
function pressure = wedge_pressure_2d( ...
    wave_number, max_bottom, tolerance, ...
    source_range, source_zeta, target_range, target_zeta )

    global wedge_angle density speed atten speed_shear atten_shear
    
    % source contribution = s(0,0) term
    
    [ range, zeta ] = spherical_add( ...
        target_range, target_zeta, target_phi, ...
        -source_range, source_zeta, 0.0 ) ;
    pressure = wedge_integ_2d( num_surface, num_bottom, ...
        wave_number, range, zeta )
    
%     % 1st surface image = s(1,0) term
%     
%     [ R, zeta ] = spherical_add( ...
%         target_range, target_zeta, target_phi, ...
%         -source_range, pi-source_zeta, 0.0 ) ;
%     [ pressure, real_spectrum, imag_spectrum ] = wedge_pressure_2d( ...
%         wave_number, real_theta, imag_theta, 1, 0, R, zeta, ...
%         wedge_angle, density, speed, atten, speed_shear, atten_shear ) ; 
%     
%     % loop through images that include bottom reflection
%     
%     for num_bottom = 1:max_bottom
%         
%         % source->surface->bottom->target path = s'(n,n) term
%         
%         [ rho, theta ] = spherical_add( ...
%             target_range, target_zeta, target_phi, ...
%             -source_range, -source_zeta-2*wedge_theta*(num_bottom-1), 0.0 ) ;
%         new = wedge_image_2d( ...
%             wave_number, wave_theta, num_bottom, num_bottom, rho, theta, ...
%             wedge_theta, density, speed, atten, speed_shear, atten_shear ) ;
%         
%         % source->bottom->target path = s'(n-1,n) term
%         
%         [ rho, theta ] = spherical_add( ...
%             target_range, target_zeta, target_phi, ...
%             -source_range, source_zeta-2*wedge_theta*num_bottom, 0.0 ) ;
%         new = new + wedge_image_2d( ...
%             wave_number, wave_theta, num_bottom-1, num_bottom, rho, theta, ...
%             wedge_theta, density, speed, atten, speed_shear, atten_shear ) ;
%         
%         % source->bottom->surface->target path = s(n,n) term
%         
%         [ rho, theta ] = spherical_add( ...
%             target_range, target_zeta, target_phi, ...
%             -source_range, -source_zeta+2*wedge_theta*num_bottom, 0.0 ) ;
%         new = new + wedge_image_2d( ...
%             wave_number, wave_theta, num_bottom, num_bottom, rho, theta, ...
%             wedge_theta, density, speed, atten, speed_shear, atten_shear ) ;
%         
%         % source->surface->bottom->surface->target path = s(n+1,n) term
%         
%         [ rho, theta ] = spherical_add( ...
%             target_range, target_zeta, target_phi, ...
%             -source_range, source_zeta+2*wedge_theta*(num_bottom+1), 0.0 ) ;
%         new = new + wedge_image_2d( ...
%             wave_number, wave_theta, num_bottom+1, num_bottom, rho, theta, ...
%             wedge_theta, density, speed, atten, speed_shear, atten_shear ) ;
%         
%         % incorporate new contribution into the sum
%         % exit early if the new contribution is small
%         
%         H = H + new ;
%         if ( max( abs(new./H)) < tolerance )
%             break ;
%         end
%         
%     end    
%     
end