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
de_index = de_index:5:length(wavefront.source_de);
[p,az_index] = min( abs(wavefront.source_az+30) ) ;
scale = [0 60 -300 0];

% plot depth vs. time for one azimuth

while ( true )

    % plot wavefront
    
	wlat = double( squeeze( wavefront.latitude(:,de_index,az_index) ) );
	wlng = double( squeeze( wavefront.longitude(:,de_index,az_index) ) );
	walt = double( squeeze( wavefront.altitude(:,de_index,az_index) ) );
	wbtm = double( squeeze( wavefront.bottom(:,de_index,az_index) ) );

    plot( wavefront.travel_time, walt ) ;
    grid; axis( scale ) ;    
    title(sprintf('Launch Azimuth = %.1f',wavefront.source_az(az_index)));
    drawnow() ;
    
    % allows user to move the plot forward and backward in time

    choice = menu('Action','Quit','Next','+10','Previous','-10','Save') ;
    switch ( choice )
        case 1
            break ;
        case 2
            if ( az_index < max_az ) az_index = az_index + 1 ; end ;
        case 3
            if ( az_index < max_az-10 ) az_index = az_index + 10 ; end ;
        case 4
            if ( az_index > 1 ) az_index = az_index - 1 ; end ;
        case 5
            if ( az_index > 10 ) az_index = az_index - 10 ; end ;
        case 6
            title('');
            print -deps run1a_vertrays
    end
    scale = axis ;      % store user changes to axes


end
