%
% Movie of wavefronts for refraction_munk_wave.
%
% Illustrates the smoothness of the wavefront and the lack of false
% caustics.  Illustrates the detection of caustics by the ray tracing 
% algorithm.
%
% Supports menu controls for next/prev wavefront.
% Maintains user selections for plot axes for next/prev wavefront.
%
clear all ; close all;

% load wavefront and convert into range/depth coordinates

wavefront = load_wavefront('refraction_munk_range.nc') ;
az_index = 1 ;
wlat = squeeze(wavefront.latitude(:,:,az_index)) ;
wlat = ( wlat - 45 ) * ( 1852.0 * 60.0 ) / 1e3 ; % range in meters
walt = squeeze(wavefront.altitude(:,:,az_index))  ;
wcst = squeeze(wavefront.caustic(:,:,az_index))  ;
max_time = length( wavefront.travel_time ) ;

% search for caustics

cst_index = zeros( size( wcst ) ) ;
for t=2:max_time
    for d=1:length(wcst(1,:)) 
        if ( wcst(t,d) > wcst(t-1,d) ) 
            cst_index(t,d) = 1.0 ;
        end
    end
end

% loop through the wavefront displays

figure; 
scale = [ 0 100 -5000 0 ] ;
[p,t_index]= min( abs(wavefront.travel_time-5.0) ) ;
while (1)
    
    % plot wavefronts and rays
    
    plot( wlat(t_index,:), walt(t_index,:), 'r-', wlat, walt, 'c-', ...
          wlat( cst_index>0 ), walt( cst_index>0 ), 'b.' ) ;
    grid;
    axis( scale ) ;
    xlabel('Range (km)');
    ylabel('Depth (m)');
    title(sprintf( 'refraction munk t(%d)=%.1f sec', ...
        t_index, wavefront.travel_time(t_index) ));
    % legend('wavefront','rays','Location','Best');
    drawnow;
    
    % allows user to move the plot forward and backward in time
    
    choice = menu('Action','Quit','Next','+10','Prev','-10') ;
    switch ( choice )
        case 1
            break ;
        case 2
            if ( t_index < max_time-10 ) t_index = t_index + 1 ; end ;
        case 3
            if ( t_index < max_time-100 ) t_index = t_index + 10 ; end ;
        case 4
            if ( t_index > 10 ) t_index = t_index - 1 ; end ;
        case 5
            if ( t_index > 100 ) t_index = t_index - 10 ; end ;
    end
    scale = axis ;      % store user changes to axes
end
