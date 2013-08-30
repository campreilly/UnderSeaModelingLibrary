% load proploss data from WaveQ3D model
% clear; clc;

deg_2_m = 1852.0 * 60.0 / 1e3 ;      % conversion factor from deg lat to km
[proploss, eigenrays] = load_proploss('analytic_wedge_proploss.nc') ;

x_val = proploss.latitude(:,1) * deg_2_m ;
y_val = -proploss.intensity(:,1) ;

% Plot transmission loss results from WaveQ3D

figure ;
plot( x_val, y_val, 'o-' ) ;
grid ;
set(gca, 'YLim', [-90 -35])
xlabel('Range (km)');
ylabel('Propagation Loss (dB)');
