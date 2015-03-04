% Zoom in on the ray paths for eigenray_lloyds_wave test.
%
% Illustrates the fold in the wavefront near the ocean surface.  It shows
% how the distance from a wavefront edge to the surface can get large at
% short ranges.  This causes inaccuracies in the model by forcing
% an increased level of extrapolation.
%
clear all ; 
close all;

% load wavefront and convert into range/depth coordinates

az = 0 ; 
wavefront = load_wavefront('eigenray_lloyds_wave.nc') ;
[p,az_index] = min( abs( wavefront.source_az - az ) ) ;
wlat = squeeze(wavefront.latitude(:,:,az_index))-45 ;
walt = squeeze(wavefront.altitude(:,:,az_index)) ;
wsrf = squeeze(wavefront.surface(:,:,az_index)) ;
[Ntimes,Nrays] = size(wlat);

% zoom in on short range rays near surface

figure; 
for ray = (Nrays-1):-1:(Nrays-50)
    n = find( wsrf(:,ray) == 0 ) ;
    m = find( wsrf(:,ray) == 1 ) ;
    k = [ n(end) m(1) ] ;
    plot( wlat(n,ray), walt(n,ray), 'k-', ...
          wlat(m,ray), walt(m,ray), 'k--', ...
          wlat(k,ray), walt(k,ray), 'k:', ...
          'LineWidth', 1.25 ) ;
    axis([0 0.025 -50 0]) ;
    hold on
end
xlabel('Latitude Change (deg)');
ylabel('Depth (m)');
legend('Direct Path','Surface Reflected','Discontinuity','Location','SouthEast')

a = get(gca,'Xlim') ;
set(gca,'XTick',a(1):0.005:a(2));
title(sprintf('%.2f km Range Window',(a(2)-a(1))*(1852.0*60.0)/1e3) ) ;

print -deps eigenray_lloyds_zoom.eps
