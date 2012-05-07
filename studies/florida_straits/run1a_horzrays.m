%%
% run1a_vertrays.m - plot a vertical cut through wavefront
%
% The user controls azimuth (forward and back) using a GUI menu.
% Used as both a debugging and demonstration tool.
%
clear all; 
close all;

% load wavefront into memory 

wavefront = load_wavefront('run1a_wavefront.nc') ;
max_az = length(wavefront.source_az) ;
[p,de_index] = min( abs(wavefront.source_de-2) ) ;
scale = [-80.1 -79.85 26 26.8] ;

% load and plot batyhymetry data

bathymetry = load_arc_ascii('flstrts_bathymetry.asc');
figure;
run1a_plot_bathy( bathymetry ) ;
title('CALOPS RUN 1N SEPT 2007');
set(gca,'ZLim',[-800 0]);
view([0 90]);
axis(scale);

hold on ;
[c,h] = contour(bathymetry.longitude,bathymetry.latitude,bathymetry.altitude,[0 -236.0]);
set(h,'LineColor','black');
hold off ;

% plot depth vs. time for one azimuth

while ( true )

    % plot wavefront
    
	wlat = double( squeeze( wavefront.latitude(:,de_index,:) ) );
	wlng = double( squeeze( wavefront.longitude(:,de_index,:) ) );
	walt = double( squeeze( wavefront.altitude(:,de_index,:) ) );
	wbtm = double( squeeze( wavefront.bottom(:,de_index,:) ) );

    hold on;
    hw = plot3( wlng, wlat, zeros(size(wlng)), 'k' ) ;
    grid; axis( scale ) ;    
    title(sprintf('Launch D/E = %.1f',wavefront.source_de(de_index)));
    hold off;
    drawnow() ;
    
    % allows user to move the plot forward and backward in time

    choice = menu('Action','Quit','Next','+10','Previous','-10','Save') ;
    switch ( choice )
        case 1
            break ;
        case 2
            if ( de_index < max_az ) de_index = de_index + 1 ; end ;
        case 3
            if ( de_index < max_az-10 ) de_index = de_index + 10 ; end ;
        case 4
            if ( de_index > 1 ) de_index = de_index - 1 ; end ;
        case 5
            if ( de_index > 10 ) de_index = de_index - 10 ; end ;
        case 6
            title('');
            print -deps run1a_horzrays
    end
    scale = axis ;      % store user changes to axes
    delete(hw) ;

end
