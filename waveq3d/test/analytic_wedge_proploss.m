% load proploss data from WaveQ3D model

[ proploss, eigenrays ] = load_proploss('analytic_wedge_proploss.nc');
[n,m] = size(proploss.intensity);
inc = 4/n;
range = (0:inc:4-inc) ;

% Plot transmission loss results from WaveQ3D

figure ;
plot( range, -proploss.intensity, '-', ...
      'LineWidth', 1.25 ) ;
grid ;
xlabel('Range (km)');
ylabel('Propagation Loss (dB)');
