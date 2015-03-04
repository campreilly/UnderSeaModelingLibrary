% Compare the eigenrays generate by GRAB and the WaveQ3D model
% for Pedersen shallow source test.
%
disp('*** pedersen_shallow_compare ***');
clear all;
close all;
earth_radius = 6378101.030201019 ; % earth radius at 45 deg north
d2r = pi / 180 ;        % converts degrees to radians
max_range = 0.91 ;
% max_range = 0.75 ;

if( exist('pedersen_eigenrays_shallow.mat', 'file') ~= 2 )
    [d, s] = pedersen_eigenrays_shallow() ;
    analytic.direct = d ;
    analytic.surface = s ;
else
    analytic = load('pedersen_eigenrays_shallow.mat');
end
direct_index = find( ~isnan(analytic.direct.travel_time) ) ;
surface_index = find( ~isnan(analytic.surface.travel_time) ) ;

% load propagation loss data from FFP

[data,desc] = xlsread('n2shallow.csv');
ffp.range = data(:,1)'/1e3 ;
ffp.intensity = data(:,2)' ;
clear data desc

% load eigenray data from GRAB
% separate into surface and upper vertex paths

[ eigenrays, grab.pressure ] = pedersen_shallow_grab() ;

n = find( eigenrays.surface == 1 ) ;
grab.surface = struct( ...
    'range',        eigenrays.range(n), ...
    'travel_time',  eigenrays.travel_time(n), ...
    'source_de',    -eigenrays.source_de(n), ...
    'target_de',    -eigenrays.target_de(n), ...
    'intensity',    eigenrays.intensity(n), ...
    'phase',        eigenrays.phase(n), ...
    'surface',      eigenrays.surface(n), ...
    'bottom',       eigenrays.bottom(n), ...
    'upper',        eigenrays.upper(n), ...
    'lower',        eigenrays.lower(n) ) ;

n = find( eigenrays.upper == 1 ) ;
grab.direct = struct( ...
    'range',        eigenrays.range(n), ...
    'travel_time',  eigenrays.travel_time(n), ...
    'source_de',    -eigenrays.source_de(n), ...
    'target_de',    -eigenrays.target_de(n), ...
    'intensity',    eigenrays.intensity(n), ...
    'phase',        eigenrays.phase(n), ...
    'surface',      eigenrays.surface(n), ...
    'bottom',       eigenrays.bottom(n), ...
    'upper',        eigenrays.upper(n), ...
    'lower',        eigenrays.lower(n) ) ;
clear eigenrays

% load eigenray data from WaveQ3D model
% separate into surface and direct paths

[ plr.proploss, eigenrays ] = load_proploss('pedersen_shallow_proploss.nc');
range = (500:1.0:1000)/1e3 ;
xscale = [range(1) range(end)] ;

n = 0 ;
m = 0 ;
for t = 1:length(eigenrays)
    for p = 1:length(eigenrays(t).travel_time)
        if ( eigenrays(t).surface(p) == 0 )
            m = m + 1 ;
            plr.direct.range(m,1) = range(t) ;
            plr.direct.travel_time(m,1) = eigenrays(t).travel_time(p) ;
            plr.direct.source_de(m,1) = eigenrays(t).source_de(p) ;
            plr.direct.target_de(m,1) = eigenrays(t).target_de(p) ;
            plr.direct.intensity(m,1) = -eigenrays(t).intensity(p) ;
            plr.direct.phase(m,1) = eigenrays(t).phase(p) / d2r ;
            plr.direct.surface(m,1) = eigenrays(t).surface(p) ;
            plr.direct.bottom(m,1) = eigenrays(t).bottom(p) ;
            plr.direct.caustic(m,1) = eigenrays(t).caustic(p) ;
        else
            n = n + 1 ;
            plr.surface.range(n,1) = range(t) ;
            plr.surface.travel_time(n,1) = eigenrays(t).travel_time(p) ;
            plr.surface.source_de(n,1) = eigenrays(t).source_de(p) ;
            plr.surface.target_de(n,1) = eigenrays(t).target_de(p) ;
            plr.surface.intensity(n,1) = -eigenrays(t).intensity(p) ;
            plr.surface.phase(n,1) = eigenrays(t).phase(p) / d2r ;
            plr.surface.surface(n,1) = eigenrays(t).surface(p) ;
            plr.surface.bottom(n,1) = eigenrays(t).bottom(p) ;
            plr.surface.caustic(n,1) = eigenrays(t).caustic(p) ;
        end
    end
end
clear eigenrays

figure(1) ; 
figure(2) ; 

% compare intensity estimates

n = find( grab.direct.range <= max_range ) ;
[ bias, dev, detcoef, lag ] = tl_stats( ...
    plr.direct.range, plr.direct.intensity, ...
    grab.direct.range(n), grab.direct.intensity(n) ) ;
fprintf('Direct: bias = %.2f dB dev = %.2f dB detcoef = %.1f%% lag = %.4f m\n',...
    bias, dev, detcoef, lag ) ;

figure(1) ;
subplot(2,2,1) ;
plot( plr.direct.range, plr.direct.intensity, '-', ...
      grab.direct.range, grab.direct.intensity, '--', ...
      'LineWidth', 1.25 ) ;
grid ;
xlabel('Range (km)');
ylabel('Propagation Loss (dB)');
set(gca,'XLim',xscale)
set(gca,'YLim',[-90 -50])

n = find( grab.surface.range <= max_range ) ;
[ bias, dev, detcoef, lag ] = tl_stats( ...
    plr.surface.range, plr.surface.intensity, ...
    grab.surface.range(n), grab.surface.intensity(n) ) ;
fprintf('Surface: bias = %.2f dB dev = %.2f dB detcoef = %.1f%% lag = %.4f m\n',...
    bias, dev, detcoef, lag ) ;

figure(2) ;
subplot(2,2,1) ;
plot( plr.surface.range, plr.surface.intensity, '-', ...
      grab.surface.range, grab.surface.intensity, '--', ...
      'LineWidth', 1.25 ) ;
grid ;
xlabel('Range (km)');
ylabel('Propagation Loss (dB)');
set(gca,'XLim',xscale)
set(gca,'YLim',[-90 -50])

% compare travel time estimates

n = find( ~isnan(analytic.direct.travel_time) ) ;
slope = ( analytic.direct.travel_time(n(end)) - analytic.direct.travel_time(1) ) ...
     / ( analytic.direct.range(n(end)) - analytic.direct.range(1) ) ;

grab.direct.travel_time = grab.direct.travel_time ...
    - analytic.direct.travel_time(1) ...
    - slope * ( grab.direct.range - grab.direct.range(1) ) ;
grab.surface.travel_time = grab.surface.travel_time ...
    - analytic.direct.travel_time(1) ...
    - slope * ( grab.surface.range - grab.surface.range(1) ) ;
plr.direct.travel_time = plr.direct.travel_time ...
    - analytic.direct.travel_time(1) ...
    - slope * ( plr.direct.range - plr.direct.range(1) ) ;
plr.surface.travel_time = plr.surface.travel_time ...
    - analytic.direct.travel_time(1) ...
    - slope * ( plr.surface.range - plr.surface.range(1) ) ;
analytic.surface.travel_time(n) = analytic.surface.travel_time(n) ...
    - analytic.direct.travel_time(n(1)) ...
    - slope * ( analytic.surface.range(n) - analytic.surface.range(1) ) ;
analytic.direct.travel_time(n) = analytic.direct.travel_time(n) ...
    - analytic.direct.travel_time(1) ...
    - slope * ( analytic.direct.range(n) - analytic.direct.range(1) ) ;

fprintf('GRAB direct: max travel_time diff = %.2f ms\n',... 
    1e3*max(abs(grab.direct.travel_time(direct_index)-analytic.direct.travel_time(direct_index)'))) ;
fprintf('WaveQ3D direct: max travel_time diff = %.2f ms\n',... 
    1e3*max(abs(plr.direct.travel_time(direct_index)-analytic.direct.travel_time(direct_index)'))) ;
fprintf('GRAB surface: max travel_time diff = %.2f ms\n',... 
    1e3*max(abs(grab.surface.travel_time(surface_index)-analytic.surface.travel_time(surface_index)'))) ;
fprintf('WaveQ3D surface: max travel_time diff = %.2f ms\n',... 
    1e3*max(abs(plr.surface.travel_time(surface_index)-analytic.surface.travel_time(surface_index)'))) ;

figure(1) ;
subplot(2,2,2) ;
plot( plr.direct.range, plr.direct.travel_time*1e3, '-', ...
      grab.direct.range, grab.direct.travel_time*1e3, '--', ...
      analytic.direct.range, analytic.direct.travel_time*1e3, ':', ...
      'LineWidth', 1.25 ) ;
grid ;
xlabel('Range (km)');
ylabel('Travel Time - Bulk (sec)');
set(gca,'XLim',xscale)
set(gca,'YLim',[-4 14])
legend('WaveQ3D','GRAB','theory');

figure(2) ;
subplot(2,2,2) ;
plot( plr.surface.range, plr.surface.travel_time*1e3, '-', ...
      grab.surface.range, grab.surface.travel_time*1e3, '--', ...
      analytic.surface.range, analytic.surface.travel_time*1e3, ':', ...
      'LineWidth', 1.25 ) ;
grid ;
xlabel('Range (km)');
ylabel('Travel Time - Bulk (ms)');
set(gca,'XLim',xscale)
set(gca,'YLim',[-4 14])
legend('WaveQ3D','GRAB','theory');

% compare source_de estimates

fprintf('GRAB direct: max source_de diff = %.2f deg\n',... 
    max(abs(grab.direct.source_de(direct_index)-analytic.direct.source_de(direct_index)'))) ;
fprintf('WaveQ3D direct: max source_de diff = %.2f deg\n',... 
    max(abs(plr.direct.source_de(direct_index)-analytic.direct.source_de(direct_index)'))) ;
fprintf('GRAB surface: max source_de diff = %.2f deg\n',... 
    max(abs(grab.surface.source_de(surface_index)-analytic.surface.source_de(surface_index)'))) ;
fprintf('WaveQ3D surface: max source_de diff = %.2f deg\n',... 
    max(abs(plr.surface.source_de(surface_index)-analytic.surface.source_de(surface_index)'))) ;

figure(1) ;
subplot(2,2,3) ;
plot( plr.direct.range, plr.direct.source_de, '-', ...
      grab.direct.range, grab.direct.source_de, '--', ...
      analytic.direct.range, analytic.direct.source_de, ':', ...
      'LineWidth', 1.25 ) ;
grid
xlabel('Range (km)');
ylabel('Source D/E (deg)');
set(gca,'XLim',xscale)
set(gca,'YLim',[8 24])

figure(2) ;
subplot(2,2,3) ;
plot( plr.surface.range, plr.surface.source_de, '-', ...
      grab.surface.range, grab.surface.source_de, '--', ...
      analytic.surface.range, analytic.surface.source_de, ':', ...
      'LineWidth', 1.25 ) ;
grid
xlabel('Range (km)');
ylabel('Source D/E (deg)');
set(gca,'XLim',xscale)
set(gca,'YLim',[8 24])

% compare target_de estimates

fprintf('GRAB direct: max target_de diff = %.2f deg\n',... 
    max(abs(grab.direct.target_de(direct_index)-analytic.direct.target_de(direct_index)'))) ;
fprintf('WaveQ3D direct: max target_de diff = %.2f deg\n',... 
    max(abs(plr.direct.target_de(direct_index)-analytic.direct.target_de(direct_index)'))) ;
fprintf('GRAB surface: max target_de diff = %.2f deg\n',... 
    max(abs(grab.surface.target_de(surface_index)-analytic.surface.target_de(surface_index)'))) ;
fprintf('WaveQ3D surface: max target_de diff = %.2f deg\n',... 
    max(abs(plr.surface.target_de(surface_index)-analytic.surface.target_de(surface_index)'))) ;

figure(1) ;
subplot(2,2,4) ;
plot( plr.direct.range, plr.direct.target_de, '-', ...
      grab.direct.range, grab.direct.target_de, '--', ...
      analytic.direct.range, analytic.direct.target_de, ':', ...
      'LineWidth', 1.25 ) ;
grid ;
xlabel('Range (km)');
ylabel('Target D/E (deg)');
set(gca,'XLim',xscale)
set(gca,'YLim',[-24 -8])

print -deps pedersen_shallow_compare1.eps

figure(2) ;
subplot(2,2,4) ;
plot( plr.surface.range, plr.surface.target_de, '-', ...
      grab.surface.range, grab.surface.target_de, '--', ...
      analytic.surface.range, analytic.surface.target_de, ':', ...
      'LineWidth', 1.25 ) ;
grid ;
xlabel('Range (km)');
ylabel('Target D/E (deg)');
set(gca,'XLim',xscale)
set(gca,'YLim',[-24 -8])

print -deps pedersen_shallow_compare2.eps

% compare coherent TL estimates from all three models

n = find( ffp.range <= max_range ) ;
[ bias, dev, detcoef, lag ] = tl_stats( ...
    grab.pressure.range, grab.pressure.level, ...
    ffp.range(n), ffp.intensity(n) ) ;
fprintf('GRAB: bias = %.2f dB dev = %.2f dB detcoef = %.1f%% lag = %.4f m\n',...
    bias, dev, detcoef, lag ) ;

[ bias, dev, detcoef, lag ] = tl_stats( ...
    range, -plr.proploss.intensity, ...
    ffp.range(n), ffp.intensity(n) ) ;
fprintf('WaveQ3D: bias = %.2f dB dev = %.2f dB detcoef = %.1f%% lag = %.4f m\n',...
    bias, dev, detcoef, lag ) ;

figure(3) ;
plot( range, -plr.proploss.intensity, '-', ...
      grab.pressure.range, grab.pressure.level, '--', ...
      ffp.range, ffp.intensity, ':', ...
      'LineWidth', 1.25 ) ;
grid ;
xlabel('Range (km)');
ylabel('Propagation Loss (dB)');
legend('WaveQ3D','GRAB','theory','Location','NorthEast');
set(gca,'XLim',xscale)
set(gca,'YLim',[-80 -40])
print -deps pedersen_shallow_compare3.eps
