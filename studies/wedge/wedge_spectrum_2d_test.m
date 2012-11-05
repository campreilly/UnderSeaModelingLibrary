%%
% Test the Deane/Buckingham 2-D wedge solution to ensure that 
% components needed for 3-D will work.
%
function wedge_spectrum_2d_test

close all

% define tuning parameters

range_inc = 1 ;
theta_num = 1025 ;
max_bottom = 0 ;
tolerance = 1e-3 ;

% define a source parameters

source_freq = 25 ;              % Hertz
source_range = 4000 ;           % meters from wedge apex
source_depth = 100 ;            % meters from surface
source_rho = sqrt( source_range.^2 + source_depth.^2 ) ;
source_theta = atan2( source_range, -source_depth ) ;

% define environmental parameters

water_depth = 200 ;             % at source range
water_speed = 1500 ;            % isovelocity water
bottom_density = 1.5 ;          % ratio of sediment/water density
bottom_speed = 1700/water_speed;% compressional sediment/water speed ratio
bottom_atten = 0.5 ;            % compressional dB/wavelength
shear_speed = 0.0 ;             % shear sediment/water speed ratio
shear_atten = 0.0 ;             % shear dB/wavelength
wedge_theta = atan(water_depth/source_range) ;

% define a grid of receivers 30 meters below the surface

range = range_inc:range_inc:3500 ;
target_range = source_range - range ;
target_depth = 30 ;
target_rho = sqrt( target_range.^2 + target_depth.^2 ) ;
target_theta = atan2( target_range, -target_depth ) ;

% for each target, compute spectrum as a function of wave number
% then integrate over wave numbers to compute the pressure

wave_number = 2 * pi * source_freq / water_speed ;
wave_theta = (0:(1/(theta_num-1)):1)*pi/2 ;
dtheta = wave_theta(2) - wave_theta(1) ;

spectrum = zeros( length(target_theta), length(wave_theta) ) ;
pressure = zeros( length(target_theta), 1 ) ;
for t = 1:length(target_theta)
    spectrum(t,:) = wedge_spectrum_2d( ...
        wave_number, wave_theta, max_bottom, tolerance, ...
        source_rho, source_theta, target_rho(t), target_theta(t), ...
        wedge_theta, bottom_density, bottom_speed, bottom_atten, shear_speed, shear_atten ) ;
    pressure(t) = sum( spectrum(t,:) ) * dtheta ; % integration
end
clear t

save wedge_spectrum_2d_test

% plot results

plot( range, 20*log10(abs(pressure)), range, -20*log10(range) );
grid ;
xlabel('Range (m)');
ylabel('Transmission Loss (dB)');
 
end
