clear all ;
[proploss, eigenrays] = load_proploss('eigenray_tl_grid.nc') ;
wavefront = load_wavefront('eigenray_tl_grid_wave.nc') ;
data = xlsread('eigenray_tl_grid.csv') ;

figure(1) ;
for i=1:50
    for j=1:50
        x((i-1)*50+j) = proploss.latitude(i,j)*180/pi ;
        y((i-1)*50+j) = proploss.longitude(i,j)*180/pi ;
        z((i-1)*50+j) = proploss.altitude(i,j) ;
        tl((i-1)*50+j) = proploss.intensity((i-1)*50+j,1) ;
        if tl((i-1)*50+j) == 300
            tl((i-1)*50+j) = 70.0 ;
        end
    end
end
scatter3( x, y, z, 3, tl, 'fill' ) ;
title( sprintf('Transmission Loss Anomaly\nFreq: %.1f\nAZ swath(%.1f): %.2f to %.2f', ...
        wavefront.frequency(1), wavefront.source_az(2) - wavefront.source_az(1), ...
        wavefront.source_az(1), wavefront.source_az(length(wavefront.source_az))) ) ;
xlabel('Latitude (degrees)') ;
ylabel('Longitude (degrees)') ;
zlabel('Altitude (meters)') ;
ylow = min(proploss.longitude*180/pi) ;
yhigh = max(proploss.longitude*180/pi) ;
set(gca, 'YLim', [ylow(1) yhigh(1)]) ;
colorbar ;
grid on ;

figure(2) ;
angle = atan( proploss.longitude(:,26) ./ proploss.latitude(:,26) ) ;
angle = angle * 180 / pi ;
for i=1:50
    tl_angle(i,1) = proploss.intensity(26+50*(i-1)) ;
end
x_vals = data(:,1) + 0.12 ;
plot( angle, tl_angle, 'ro', ...
      x_vals, data(:,4), 'bo' ) ;
set(gca, 'YDir', 'rev') ;
set(gca, 'XLim', [-3 3]) ;
title( sprintf('Transmission Loss Anomaly\nFreq: %.1f\nAZ swath(%.1f): %.2f to %.2f', ...
        wavefront.frequency(1), wavefront.source_az(2) - wavefront.source_az(1), ...
        wavefront.source_az(1), wavefront.source_az(length(wavefront.source_az))) ) ;
xlabel('Azimuthal Angle (degrees)') ;
ylabel('Transmission Loss (dB)') ;
grid on ;

figure(3) ;
surf_angle = atan( proploss.longitude ./ proploss.latitude ) ;
surf_angle = surf_angle * 180 / pi ;
for i=1:50
    for j=1:50
        surf_tl(i,j) = proploss.intensity((i-1)*50+j) ;
        truth_tl(i,j) = data(1,4) ;
    end
end
surf( surf_angle, proploss.altitude, surf_tl, 'EdgeColor', 'none', 'Facecolor', 'interp' ) ;
hold on ;
surf( surf_angle, proploss.altitude, truth_tl, 'EdgeColor', 'none', 'Facecolor', 'interp' ) ;
hold off ;
colormap(coldhot) ;
colorbar ;
title( sprintf('Transmission Loss Anomaly\nFreq: %.1f\nAZ swath(%.1f): %.2f to %.2f', ...
        wavefront.frequency(1), wavefront.source_az(2) - wavefront.source_az(1), ...
        wavefront.source_az(1), wavefront.source_az(length(wavefront.source_az))) ) ;
xlabel('Azimuthal Angle (degres)') ;
ylabel('Altitude (meters)') ;
zlabel('Transmission Loss (dB)') ;
set(gca, 'XLim', [-3 3]) ;
grid on ;