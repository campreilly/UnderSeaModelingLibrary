% florida_wavefront.m 
%
%
clear all; 
close all;

% load wavefront into memory 

wavefront = load_wavefront('florida_wavefront.nc') ;
max_time = min(9999,length(wavefront.travel_time)) ;
tindex = 1 ;

% load and plot bathymetry data

bathymetry = load_arc_ascii('florida-crm.asc');

figure;
colormap(copper(128));
hb = surf(bathymetry.longitude,bathymetry.latitude,bathymetry.altitude,...
     'FaceColor','interp','LineStyle','none');
axis([-80.4 -79.6 26 26.8]);
set(gca,'TickDir','out');
set(gca,'CLim',[-350 0]);
set(gca,'ZLim',[-800 0]);
view([10 40]); 
xlabel('Latitude (deg)');
ylabel('Longitude (deg)');
colorbar;

% plot the receiver and source locations

hold on;
plot3(-79.99054,26.0217,-250,'o',...
    'MarkerEdgeColor','yellow','MarkerFaceColor','yellow');

x = [ -79.99054 -79.88057 ] ;
y = [ 26.03545 26.73028 ] ;
z = [ -100 -100 ];
plot3(x,y,z,'Color','white');
hold off

% show a movie of wavefront propagation

hw = 0 ;
% while ( true )

    % plot wavefront
    
	wlng = double( squeeze( wavefront.longitude(tindex,:,:) ) );
	wlat = double( squeeze( wavefront.latitude(tindex,:,:) ) );
	walt = double( squeeze( wavefront.altitude(tindex,:,:) ) );
    
	hw = surf( wlng, wlat, walt ) ;
	% set(hw,'FaceColor','white');
	% set(hw,'EdgeColor','black');
    title(sprintf('Travel Time = %.1f',wavefront.travel_time(tindex)));
    drawnow() ;
    
    % get ready for next plot

%     tindex = tindex + 3 ;
%     if ( tindex > max_time ) break; end ;
%     pause(0.12);
% 	delete(hw) ;

% end