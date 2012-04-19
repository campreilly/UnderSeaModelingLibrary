% florida_wavefront.m 
%
%
clear all; 
close all;

% load wavefront into memory 

wavefront = load_wavefront('run1a_wavefront.nc') ;
max_time = min(9999,length(wavefront.travel_time)) ;
tindex = 1 ;

% load and plot batyhymetry data

bathymetry = load_arc_ascii('flstrts_bathymetry.asc');
figure;
run1a_plot_bathy( bathymetry ) ;
title('CALOPS RUN 1N SEPT 2007');
set(gca,'ZLim',[-800 0]);
view([10 40]); 

% show a movie of wavefront propagation

% hw = 0 ;
% % while ( true )
% 
%     % plot wavefront
%     
% 	wlng = double( squeeze( wavefront.longitude(tindex,:,:) ) );
% 	wlat = double( squeeze( wavefront.latitude(tindex,:,:) ) );
% 	walt = double( squeeze( wavefront.altitude(tindex,:,:) ) );
%     
% 	hw = surf( wlng, wlat, walt ) ;
% 	% set(hw,'FaceColor','white');
% 	% set(hw,'EdgeColor','black');
%     title(sprintf('Travel Time = %.1f',wavefront.travel_time(tindex)));
%     drawnow() ;
%     
%     % get ready for next plot
% 
% %     tindex = tindex + 3 ;
% %     if ( tindex > max_time ) break; end ;
% %     pause(0.12);
% % 	delete(hw) ;
% 
% % end
