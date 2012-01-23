%
% Movie of wavefronts for pedersen_deep_raytrace.
% Illustrates the effect of caustic on wavefront folds.
% Also illustrate impact of wavefront on deep targets.
%
% Supports menu controls for next/prev wavefront.
% Maintains user selections for plot axes for next/prev wavefront.
% Plots range and depth in yards for easier comparison to Pederson results.
%
% clear all ; 
close all;
earth_radius=6378101.030201019;
d2r = pi / 180 ;
time_init = 2.5 ;

% load wavefront and convert into range/depth coordinates

wavefront = load_wavefront('pedersen_deep_raytrace.nc') ;
de_index = 1:1:length(wavefront.source_de) ;
% de_index = (260:279)+1 ;
wlat = double( wavefront.latitude(:,de_index) ) ;
wlat = (wlat-45)*d2r * earth_radius / 1e3 ;             % range in km
walt = double( wavefront.altitude(:,de_index) ) ;       % depth in meters
wsrf = double( wavefront.surface(:,de_index) ) ;
wcst = double( wavefront.caustic(:,de_index) ) ;

% define target locations

xtarget = (3000:0.25:3100)/1e3 ;
ytarget = -800.0 ;
scale = [ 0 3.5 -1200 0 ] ;

% loop through the wavefront displays

figure; 
max_time = length( wavefront.travel_time ) ;
[p,time_index]= min( abs(wavefront.travel_time-time_init) ) ;
while (1)
    
    % find direct, caustic, and surface reflected paths
    
    ndirect = find( wcst(time_index,:) == 0 & wsrf(time_index,:) == 0 ) ;
    ncaustic = find( wcst(time_index,:) == 1 ) ;
    nsurf = find( wsrf(time_index,:) == 1 ) ;
    
    % plot wavefronts and rays

    plot( wlat, walt, 'c-', ...
          xtarget, ytarget, 'k.' ) ;
    hold on;
    h = plot( wlat(time_index,ndirect), walt(time_index,ndirect), 'bo-', ...
          wlat(time_index,nsurf), walt(time_index,nsurf), 'gs-', ...
          wlat(time_index,ncaustic), walt(time_index,ncaustic), 'rd-' ) ;
    for n = 3:length(h)
        set(h(n),'MarkerFaceColor',get(h(n),'Color'));
    end
    legend(h,'direct','surface','caustic','Location','NorthEast');
    hold off
    grid; axis( scale ) ;
    xlabel('Range (km)');
    ylabel('Depth (m)');
    title(sprintf( 'pederson deep raytrace t(%d)=%.0f msec', ...
        time_index, 1000*wavefront.travel_time(time_index) ));
    drawnow;
    
    % allows user to move the plot forward and backward in time

    choice = menu('Action','Quit','Next','+10','Previous','-10','Save') ;
    switch ( choice )
        case 1
            break ;
        case 2
            if ( time_index < max_time ) time_index = time_index + 1 ; end ;
        case 3
            if ( time_index < max_time-10 ) time_index = time_index + 10 ; end ;
        case 4
            if ( time_index > 1 ) time_index = time_index - 1 ; end ;
        case 5
            if ( time_index > 10 ) time_index = time_index - 10 ; end ;
        case 6
            title('');
            print -deps pedersen_deep_raytrace
    end
    scale = axis ;      % store user changes to axes
end
