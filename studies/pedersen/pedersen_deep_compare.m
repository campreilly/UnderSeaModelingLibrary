% Compare the eigenrays generate by GRAB and the WaveQ3D model
% for Pedersen deep source test.
%
disp('*** pedersen_deep_compare ***');
clear all;
close all;
earth_radius = 6378101.030201019 ; % earth radius at 45 deg north
d2r = pi / 180 ;        % converts degrees to radians
min_range = 0 ;
% min_range = 3.04 ;

if( exist('pedersen_eigenrays_deep.mat', 'file') ~= 2 )
    [d, s] = pedersen_eigenrays_deep() ;
    analytic.direct = d ;
    analytic.caustic = s ;
else
    analytic = load('pedersen_eigenrays_deep.mat');
end
direct_index = find( ~isnan(analytic.direct.travel_time) ) ;
caustic_index = find( ~isnan(analytic.caustic.travel_time) ) ;

% load propagation loss data from FFP

sheet=importdata('ffp_n2deep.csv',',');
data = sheet ;
ffp.range = data(:,1)'/1e3 ;
ffp.intensity = data(:,2)' ;
clear data

% load eigenray data from GRAB
% separate into caustic and upper vertex paths

[ eigenrays, grab.pressure ] = pedersen_deep_grab() ;

n = find( eigenrays.upper == 1 & eigenrays.phase == 0.0 ) ;
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

n = find( eigenrays.upper == 1 & eigenrays.phase ~= 0.0 ) ;
grab.caustic = struct( ...
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
% separate into caustic and direct paths
% ignore surface reflected path because it has only shadow zone solutions

[ plr.proploss, eigenrays ] = load_proploss('pedersen_deep_proploss.nc');
range = (3000:0.25:3120)/1e3 ;
xscale = [range(1) range(end)] ;

n = 0 ;
m = 0 ;
for t = 1:length(eigenrays)
    for p = 1:length(eigenrays(t).travel_time)
        if ( eigenrays(t).caustic(p) == 0 & eigenrays(t).surface(p) == 0 )
            m = m + 1 ;
            plr.direct.range(m,1) = range(t) ;
            plr.direct.travel_time(m,1) = eigenrays(t).travel_time(p) ;
            plr.direct.source_de(m,1) = eigenrays(t).source_de(p) ;
            plr.direct.target_de(m,1) = eigenrays(t).target_de(p) ;
            plr.direct.intensity(m,1) = -eigenrays(t).intensity(p) ;
            plr.direct.phase(m,1) = eigenrays(t).phase(p) / d2r ;
            plr.direct.caustic(m,1) = eigenrays(t).caustic(p) ;
            plr.direct.bottom(m,1) = eigenrays(t).bottom(p) ;
            plr.direct.caustic(m,1) = eigenrays(t).caustic(p) ;
        elseif ( eigenrays(t).caustic(p) == 1)
            n = n + 1 ;
            plr.caustic.range(n,1) = range(t) ;
            plr.caustic.travel_time(n,1) = eigenrays(t).travel_time(p) ;
            plr.caustic.source_de(n,1) = eigenrays(t).source_de(p) ;
            plr.caustic.target_de(n,1) = eigenrays(t).target_de(p) ;
            plr.caustic.intensity(n,1) = -eigenrays(t).intensity(p) ;
            plr.caustic.phase(n,1) = eigenrays(t).phase(p) / d2r ;
            plr.caustic.caustic(n,1) = eigenrays(t).caustic(p) ;
            plr.caustic.bottom(n,1) = eigenrays(t).bottom(p) ;
            plr.caustic.caustic(n,1) = eigenrays(t).caustic(p) ;
        end
    end
end
clear eigenrays

figure(1) ; 
figure(2) ; 

% compare intensity estimates

n = find( grab.direct.range >= min_range ) ;
[ bias, dev, detcoef ] = tl_stats( ...
    plr.direct.range, plr.direct.intensity, ...
    grab.direct.range(n), grab.direct.intensity(n) ) ;
fprintf('Direct: bias = %.2f dB dev = %.2f dB detcoef = %.1f%%\n',...
    bias, dev, detcoef ) ;

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

n = find( grab.caustic.range >= min_range ) ;
[ bias, dev, detcoef ] = tl_stats( ...
    plr.caustic.range, plr.caustic.intensity, ...
    grab.caustic.range(n), grab.caustic.intensity(n) ) ;
fprintf('Caustic: bias = %.2f dB dev = %.2f dB detcoef = %.1f%%\n',...
    bias, dev, detcoef ) ;

figure(2) ;
subplot(2,2,1) ;
plot( plr.caustic.range, plr.caustic.intensity, '-', ...
      grab.caustic.range, grab.caustic.intensity, '--', ...
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
grab.caustic.travel_time = grab.caustic.travel_time ...
    - analytic.direct.travel_time(1) ...
    - slope * ( grab.caustic.range - grab.caustic.range(1) ) ;
plr.direct.travel_time = plr.direct.travel_time ...
    - analytic.direct.travel_time(1) ...
    - slope * ( plr.direct.range - plr.direct.range(1) ) ;
plr.caustic.travel_time = plr.caustic.travel_time ...
    - analytic.direct.travel_time(1) ...
    - slope * ( plr.caustic.range - plr.caustic.range(1) ) ;
analytic.caustic.travel_time(n) = analytic.caustic.travel_time(n) ...
    - analytic.direct.travel_time(n(1)) ...
    - slope * ( analytic.caustic.range(n) - analytic.caustic.range(1) ) ;
analytic.direct.travel_time(n) = analytic.direct.travel_time(n) ...
    - analytic.direct.travel_time(1) ...
    - slope * ( analytic.direct.range(n) - analytic.direct.range(1) ) ;

fprintf('GRAB direct: max travel_time diff = %.2f ms\n',... 
    1e3*max(abs(grab.direct.travel_time(direct_index)-analytic.direct.travel_time(direct_index)'))) ;
fprintf('WaveQ3D direct: max travel_time diff = %.2f ms\n',... 
    1e3*max(abs(plr.direct.travel_time(direct_index)-analytic.direct.travel_time(direct_index)'))) ;
fprintf('GRAB caustic: max travel_time diff = %.2f ms\n',... 
    1e3*max(abs(grab.caustic.travel_time(caustic_index)-analytic.caustic.travel_time(caustic_index)'))) ;
fprintf('WaveQ3D caustic: max travel_time diff = %.2f ms\n',... 
    1e3*max(abs(plr.caustic.travel_time(caustic_index)-analytic.caustic.travel_time(caustic_index)'))) ;

figure(1) ;
subplot(2,2,2) ;
plot( plr.direct.range, plr.direct.travel_time*1e3, '-', ...
      grab.direct.range, grab.direct.travel_time*1e3, '--', ...
      analytic.direct.range, analytic.direct.travel_time*1e3, ':', ...
      'LineWidth', 1.25 ) ;
grid ;
xlabel('Range (km)');
ylabel('Travel Time - Bulk (ms)');
set(gca,'XLim',xscale)
set(gca,'YLim',[-4 14])
legend('WaveQ3D','GRAB','theory');

figure(2) ;
subplot(2,2,2) ;
plot( plr.caustic.range, plr.caustic.travel_time*1e3, '-', ...
      grab.caustic.range, grab.caustic.travel_time*1e3, '--', ...
      analytic.caustic.range, analytic.caustic.travel_time*1e3, ':', ...
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
fprintf('GRAB caustic: max source_de diff = %.2f deg\n',... 
    max(abs(grab.caustic.source_de(caustic_index)-analytic.caustic.source_de(caustic_index)'))) ;
fprintf('WaveQ3D caustic: max source_de diff = %.2f deg\n',... 
    max(abs(plr.caustic.source_de(caustic_index)-analytic.caustic.source_de(caustic_index)'))) ;

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
set(gca,'YLim',[36 52])

figure(2) ;
subplot(2,2,3) ;
plot( plr.caustic.range, plr.caustic.source_de, '-', ...
      grab.caustic.range, grab.caustic.source_de, '--', ...
      analytic.caustic.range, analytic.caustic.source_de, ':', ...
      'LineWidth', 1.25 ) ;
grid
xlabel('Range (km)');
ylabel('Source D/E (deg)');
set(gca,'XLim',xscale)
set(gca,'YLim',[36 52])

% compare target_de estimates

fprintf('GRAB direct: max target_de diff = %.2f deg\n',... 
    max(abs(grab.direct.target_de(direct_index)-analytic.direct.target_de(direct_index)'))) ;
fprintf('WaveQ3D direct: max target_de diff = %.2f deg\n',... 
    max(abs(plr.direct.target_de(direct_index)-analytic.direct.target_de(direct_index)'))) ;
fprintf('GRAB caustic: max target_de diff = %.2f deg\n',... 
    max(abs(grab.caustic.target_de(caustic_index)-analytic.caustic.target_de(caustic_index)'))) ;
fprintf('WaveQ3D caustic: max target_de diff = %.2f deg\n',... 
    max(abs(plr.caustic.target_de(caustic_index)-analytic.caustic.target_de(caustic_index)'))) ;

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
set(gca,'YLim',[-52 -36])
print -deps pedersen_deep_compare1.eps

figure(2) ;
subplot(2,2,4) ;
plot( plr.caustic.range, plr.caustic.target_de, '-', ...
      grab.caustic.range, grab.caustic.target_de, '--', ...
      analytic.caustic.range, analytic.caustic.target_de, ':', ...
      'LineWidth', 1.25 ) ;
grid ;
xlabel('Range (km)');
ylabel('Target D/E (deg)');
set(gca,'XLim',xscale)
set(gca,'YLim',[-52 -36])
print -deps pedersen_deep_compare2.eps

% compare coherent TL estimates from all three models

n = find( ffp.range >= min_range ) ;
[ bias, dev, detcoef ] = tl_stats( ...
    grab.pressure.range, grab.pressure.level, ...
    ffp.range(n), ffp.intensity(n) ) ;
fprintf('GRAB: bias = %.2f dB dev = %.2f dB detcoef = %.1f%%\n',...
    bias, dev, detcoef ) ;

[ bias, dev, detcoef ] = tl_stats( ...
    range, -plr.proploss.intensity, ...
    ffp.range(n), ffp.intensity(n) ) ;
fprintf('WaveQ3D: bias = %.2f dB dev = %.2f dB detcoef = %.1f%%\n',...
    bias, dev, detcoef ) ;

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
print -deps pedersen_deep_compare3.eps
