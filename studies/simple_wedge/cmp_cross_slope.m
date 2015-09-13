%% plot_simple_wedge
% Compare analystic solution for transmission loss, as a function of range
% across the slope, using a simple 3-D wedge, to an equivalent
% solution for a 2-D environment of constant depth.
%
function cmp_cross_slope( ...
    freq, ...           % frequency (Hz)
    angle_wedge, ...    % wedge angle (deg)
    range_src, ...      % horizontal range of source from apex (m)
    depth_src, ...      % depth of source from ocean surface (m)
    cross_rcv )         % horizontal range of receiver across slopw (m)

%% define scenerio geometry

sound_speed = 1500 ;                        % speed of sound in water (m/s)
wave_number = 2 * pi * freq / sound_speed ; % acoustic wave number (1/m)
angle_wedge = angle_wedge * pi/180.0 ;      % wedge angle (radians)
range_src = sqrt(range_src^2+depth_src^2) ; % slant range from apex (m)
angle_src = atan(depth_src/range_src) ;     % source half way to bottom (ASA value)
range_rcv = range_src ;                     % receiver range matches source
angle_rcv = angle_src ;                     % receiver angle matches source

%% compute wedge transmission loss

pressure = simple_wedge( wave_number, angle_wedge, ...
    range_rcv, angle_rcv, cross_rcv, range_src, angle_src ) ;
wedge = 20 * log10( abs(pressure) ) ;

%% compute flat bottom transmission loss

pressure = simple_flat( wave_number, angle_wedge, ...
    range_rcv, angle_rcv, cross_rcv, range_src, angle_src ) ;
flat = 20 * log10( abs(pressure) ) ;

%% plot results

figure;
h = plot( cross_rcv/1e3, flat, '-', ...
          cross_rcv/1e3, wedge, '-', ...
          'LineWidth', 1 ) ;
grid;
xlabel('Cross Slope Range (km)');
ylabel('Transmission Loss (dB)');
title(sprintf('%.0f Hz %.2f^o wedge',freq,angle_wedge*180/pi));
set(gca,'YLim',[-160 -60]);
% set(gca,'XLim',[0 4]);
legend('Flat Bottom','3-D Wedge')

