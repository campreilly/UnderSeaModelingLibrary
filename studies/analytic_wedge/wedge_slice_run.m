%%
% Compute a 2-D slice of transmission loss
%
function wedge_slice

% define tuning parameters

range_inc = 500 ;
phi_inc = 1.0 * pi / 180 ;

% define ASA wedge scenario

source_freq = 25 ;              % Hertz
source_range = 4000 ;           % meters from wedge apex
source_depth = 100 ;            % meters from surface
water_depth = 200 ;             % at source range
water_speed = 1500 ;            % isovelocity water
bottom_speed = 1700/water_speed;% ratio of sediment/water sound speed
bottom_density = 1.5 ;          % ratio of sediment/water density
bottom_atten = 0.5 ;            % compressional dB/wavelength
wedge_theta = atan(water_depth/source_range) ;

% define a grid of receivers 30 meters brlow the bottom

range = 0:range_inc:3500 ;
x = source_range - range ;
y = x .* tan(wedge_theta) + 30 ;

target_range = sqrt( x.*x + y.*y ) ;
target_theta = atan( y ./ x ) ;
target_phi = 0.0 ;

% define a grid of wavenumbers

wave_number = 2 * pi * source_freq / water_speed ;
wave_theta = 0.0 ;
wave_phi = 0:phi_inc:(2*pi) ;


end
