%% cmp_simple_wedge
% Compute analystic solution for transmission loss as a function of range
% across the slope using a simple 3-D wedge. This wedge is similar to
% the ASA benchmark, but the bottom reflection coefficient has been 
% simplified to +1, which allows us to compute an analytic solution.
%
% This implementation also oomputes an analystic solution for an equivalent
% 2-D geometry.  The comparison between the 2-D and 3-D solutions
% illustrates the value of using a 3-D model like WaveQ3D.
%
% Unlike the ASA benchmark, which computes results at 25 Hz, this
% implementation computes results at 2000 Hz for comparison to WaveQ3D. The
% 2000 Hz choice was made to match the conditions of the Pedersen shadow
% zone tests for a strong downwardly refracting n^2 linear environment.
%
% ref: F. B. Jensen and C. M. Ferla, "Numerical solutions of 
% range?dependent benchmark problems in ocean acoustics," 
% J. Acoust. Soc. Am., vol. 87, no. 4, pp. 1499-1510, 1990.
%
% ref: M. A. Pedersen, D. F. Gordon, Normal-Mode and Ray Theory Applied to 
% Underwater Acoustic conditions of Extreme Downward Refraction, 
% J. Acoust. Soc. Amer. 51 (1972) 323.
%
clear ; close all

%% define scenerio geometry

freq = 2000 ;                       % frequency in Hz (Pedersen)
sound_speed = 1500 ;                % speed of sound in water (ASA value)
wave_number = 2 * pi * freq / sound_speed ;
angle_wedge = 21.0375 * pi/180.0 ;  % wedge angle (ASA value)
range_src = sqrt(520^2+100^2) ;     % source slant range from apex (ASA value)
angle_src = angle_wedge / 2.0 ;     % source half way to bottom (ASA value)
range_rcv = range_src ;             % receiver angle matches source
cross_rcv = 10:10:4000 ;            % distance across slope
angle_rcv = angle_src ;             % receiver angle matches source
coherent  = false ;

%% compute wedge transmission loss

[ pressure, eigenrays ] = simple_wedge( wave_number, angle_wedge, ...
    range_rcv, angle_rcv, cross_rcv, range_src, angle_src, coherent ) ;
wedge = 10 * log10( abs(pressure) ) ;
eigenrays(:,1:2) = abs( eigenrays(:,1:2) ) ;
eigenrays(:,6) = eigenrays(:,6) / sound_speed ;
csvwrite('cmp_simple_wedge.csv',eigenrays) ;

%% compute flat bottom transmission loss

pressure = simple_flat( wave_number, angle_wedge, ...
    range_rcv, angle_rcv, cross_rcv, range_src, angle_src, coherent ) ;
flat = 10 * log10( abs(pressure) ) ;

%% load WaveQ3D results

[ proploss, eigenrays ] = load_proploss('simple_wedge_eigenray.nc');
deg2m = ( 1852.0 * 60.0 ) ;
wq_range = proploss.longitude * deg2m ;
wq_level = -proploss.intensity ;
r = 0 ;
for n=1:length(eigenrays)
    rays = eigenrays(n) ;
    for m=1:length(rays.intensity)
        r = r + 1 ;
        table(r,1) = double(rays.bottom(m)) ;   % number of bottom bounces
        table(r,2) = double(rays.surface(m)) ;  % number of surface bounces
        table(r,3) = 0.0 ;                      % down-range distance from apex
        table(r,4) = 0.0 ;                      % angle down from surface
        table(r,5) = wq_range(n) ;              % distance across slope
        table(r,6) = rays.travel_time(m);       % travel time 
        table(r,7) = rays.target_de(m) ;        % theta angle
        table(r,8) = rays.target_az(m) ;        % phi angle
        table(r,9) = -rays.intensity(m) ;       % transmission loss (dB)
    end
end
csvwrite('simple_wedge_eigenray.csv',table) ;

%% plot results

figure;
h = plot( cross_rcv/1e3, flat, 'k:', ...
          cross_rcv/1e3, wedge, 'k-.', ...
          wq_range/1e3, wq_level, 'k-', ...
          'LineWidth', 1 ) ;
% h = plot( cross_rcv/1e3, flat, ...
%           cross_rcv/1e3, wedge, ...
%           wq_range/1e3, wq_level, ...
%           'LineWidth', 1 ) ;
grid;
xlabel('Cross Slope Range (km)');
ylabel('Incoherent Transmission Loss (dB)');
title(sprintf('%.0f Hz',freq));
set(gca,'YLim',[-70 -30]);
set(gca,'XLim',[0 4]);
legend('Flat Bottom','21^o Wedge','WaveQ3D')
