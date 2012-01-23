%
% Movie of wavefronts for proploss_lloyds_range_wave.
% Illustrates the effect of surface reflection on wavefront folds.
% Also illustrates reasons for problems in the nearfield.
%
% Supports menu controls for next/prev wavefront.
% Maintains user selections for plot axes for next/prev wavefront.
%
clear all ; close all;

% load wavefront and convert into range/depth coordinates

figure; 
az = 0 ; 
wavefront = load_wavefront('proploss_lloyds_range_wave.nc') ;
[p,az_index] = min( abs( wavefront.source_az - az ) ) ;
wlat = squeeze(wavefront.latitude(:,:,az_index)) ;
wlat = ( wlat - 45 ) * ( 1852.0 * 60.0 ) /1e3 ; % range in km
walt = squeeze(wavefront.altitude(:,:,az_index)) ;
wsrf = squeeze(wavefront.surface(:,:,az_index)) ;

% define target locations

xtarget = (200:10:10e3)/1e3 ;
ytarget = -25 ;

% loop through the wavefront displays

max_time = length( wavefront.travel_time ) ;
scale = [ 0 3 -50 0 ] ;
t_index = 2 ;
while (1)
    
    % find direct and surface reflected paths
    % find invalid points between them
    
    ndirect = find( wsrf(t_index,:) == 0 ) ;
    nsurface = find( wsrf(t_index,:) == 1 ) ;
    if ( length(nsurface) == 0 )
        nfold = nsurface ;
    else
        nfold = [ (nsurface(1)-1) nsurface(1) ] ;
    end
    
    % plot wavefronts and rays
    
    plot( wlat(t_index,ndirect), walt(t_index,ndirect), 'b-', ...
          wlat(t_index,nsurface), walt(t_index,nsurface), 'g-', ...
          wlat(t_index,nfold), walt(t_index,nfold), 'r-', ...
          wlat, walt, 'c-', ...
          xtarget, ytarget, 'k.', ...
          wlat(t_index,ndirect), walt(t_index,ndirect), 'bo', ...
          wlat(t_index,nsurface), walt(t_index,nsurface), 'go' ) ;
    axis( scale ) ;
    xlabel('Range (km)');
    ylabel('Depth (m)');
    title(sprintf( 'proploss lloyds range t(%d)=%.0f msec', ...
        t_index, 1000*wavefront.travel_time(t_index) ));
    legend('direct','reflected','fold','rays','Location','SouthEast');
    drawnow;
    
    % allows user to move the plot forward and backward in time
    
    choice = menu('Action','Quit','Next','+10','Previous','-10') ;
    switch ( choice )
        case 1
            break ;
        case 2
            if ( t_index < max_time ) t_index = t_index + 1 ; end ;
        case 3
            if ( t_index < max_time-10 ) t_index = t_index + 10 ; end ;
        case 4
            if ( t_index > 1 ) t_index = t_index - 1 ; end ;
        case 5
            if ( t_index > 10 ) t_index = t_index - 10 ; end ;
    end
    scale = axis ;      % store user changes to axes
end
