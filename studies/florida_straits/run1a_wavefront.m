%%
% run1a_wavefront.m - movie of wavefront propagation
%
% Graph the wavefront as a user controlled movie.  
% The user controls time (forward and back) using a GUI menu.
% Used as both a debugging and demostration tool.
%
clear all; 
close all;

% load wavefront into memory 

wavefront = load_wavefront('run1a_wavefront.nc') ;
max_time = min(9999,length(wavefront.travel_time)) ;
time_index = 2 ;

% load and plot batyhymetry data

bathymetry = load_arc_ascii('flstrts_bathymetry.asc');
figure;
run1a_plot_bathy( bathymetry ) ;
title('CALOPS RUN 1N SEPT 2007');
set(gca,'ZLim',[-800 0]);
% view([10 40]); 
view([0 90]);
% set(gca,'YLim',[26.0 26.1])
% set(gca,'XLim',[-80.00 -79.98])

% show a movie of wavefront propagation

while ( true )

    % plot wavefront
    
	wlng = double( squeeze( wavefront.longitude(time_index,:,:) ) );
	wlat = double( squeeze( wavefront.latitude(time_index,:,:) ) );
	walt = double( squeeze( wavefront.altitude(time_index,:,:) ) );

    hold on;
	hw = surf( wlng, wlat, walt ) ;
	set(hw,'FaceColor','white');
	set(hw,'EdgeColor','black');
    title(sprintf('Travel Time = %.1f',wavefront.travel_time(time_index)));
    hold off;
    drawnow() ;
    
    % allows user to move the plot forward and backward in time

    inc = 10 ;
    choice = menu('Action','Quit','Next','+10','Previous','-10','Save') ;
    switch ( choice )
        case 1
            break ;
        case 2
            if ( time_index < max_time ) time_index = time_index + 1 ; end ;
        case 3
            if ( time_index < max_time-inc ) time_index = time_index + inc ; end ;
        case 4
            if ( time_index > 1 ) time_index = time_index - 1 ; end ;
        case 5
            if ( time_index > inc ) time_index = time_index - inc ; end ;
        case 6
            title('');
            print -deps run1a_wavefront
    end
    delete(hw) ;

end
