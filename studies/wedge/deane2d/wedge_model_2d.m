%%
% Computes complex pressure in the Deane/Buckingham wedge solution for a
% 2-D slice in the plane of of the source.  The scenario is based on
% Figure 6 in the March 1993 paper.
%
% The model is structured as follows:
%   - wedge_spectrum_2d() computes the wave number spectrum for one image.
%     This calculation uses reflection() to model bottom loss.
%   - wedge_integ_2d() uses simpson() to integrate the wave number spectrum 
%     along the contour 0 to pi/2-Inf.
%   - wedge_pressure_2d() coherently sums the contributions from all images.
%     This calculation uses spherical_add() to compute the location
%     of targets relative to each image source.
%
function wedge_model_2d
    close all ; clc
    disp('=== wedge_model_2d ===')

    global wedge_angle density speed atten speed_shear atten_shear

    % define tuning parameters

    range_inc = 500 ;
    max_bottom = 1 ;
    tolerance = 0.1 ;

    % define ASA wedge scenario

    source_freq = 25 ;          % Hertz
    water_depth = 200 ;         % at source range
    water_speed = 1500 ;        % isovelocity water
    speed = 1700/water_speed;   % ratio of sediment/water sound speed
    density = 1.5 ;             % ratio of sediment/water density
    atten = 0.5 ;               % compressional dB/wavelength
    speed_shear = 0.0 ;         % shear sound speed ration
    atten_shear = 0.0 ;         % shear attenuation
    
    % compute source range from apex and D/E relative to ocean bottom
    
    source_range = 4000 ;       % meters from wedge apex
    source_depth = 100 ;        % meters from surface
    source_zeta = atan2( source_depth, source_range ) ;
    source_range = sqrt( source_range*source_range + source_depth*source_depth ) ;
    
    % define a grid of targets 30 meters below the ocean surface

    target_range = 500:range_inc:source_range ; % meters from wedge apex
    target_range = 1000 ;       % for debug only!!!
    target_depth = 30 ;         % meters from surface
    target_zeta = atan2( source_depth, source_range ) ;
    target_range = sqrt( target_range*target_range + target_depth*target_depth ) ;

    % compute complex pressure as a function of target range
    
    wedge_angle = atan(water_depth/source_range) ;
    wave_number = 2 * pi * source_freq / water_speed ;

    pressure = wedge_pressure_2d( ...
        wave_number, max_bottom, tolerance, ...
        source_range, source_zeta, target_range, target_zeta )

end
