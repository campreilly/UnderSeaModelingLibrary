%
% Movie of wavefronts for eigenray_pedersen.
% Illustrates the effect of caustic on wavefront folds.
% Also illustrate impact of wavefront on deep targets.
%
% Supports menu controls for next/prev wavefront.
% Maintains user selections for plot axes for next/prev wavefront.
% Plots range and depth in yards for easier comparison to Pederson results.
%
clear all ; 
close all;
earth_radius=6378101.030201019;
d2r = pi / 180 ;
y2m = 0.9114 ;

% load wavefront and convert into range/depth coordinates

wavefront = load_wavefront('eigenray_pedersen_wave.nc') ;
de_index = 1:4:length(wavefront.source_de) ;
wlat = double( wavefront.latitude(:,de_index) ) ;
wlat = (wlat-45)*d2r * earth_radius ;
walt = double( wavefront.altitude(:,de_index) ) ;
wsrf = double( wavefront.surface(:,de_index) ) ;
wcst = double( wavefront.caustic(:,de_index) ) ;

% define target locations

xtarget = 3000:3100 ;
ytarget = -1000.0 ;
scale = [ 0 3600 -1600 0 ] ;

% make B/W copy for the report

figure ;
plot( wlat, walt, 'k-', xtarget, ytarget, 'k.' ) ;
grid; 
axis(scale) ;
xlabel('Range (m)');
ylabel('Depth (m)');
drawnow;
print -deps pedersen_deep_raytrace

% loop through the wavefront displays

figure; 
max_time = length( wavefront.travel_time ) ;
[p,t_index]= min( abs(wavefront.travel_time-2.66) ) ;
while (1)
    
    % find direct, caustic, and surface reflected paths
    % find folded points between them
    
    ndirect = find( wcst(t_index,:) == 0 & wsrf(t_index,:) == 0 ) ;
    ncaustic = find( wcst(t_index,:) == 1 ) ;
    nsurface = find( wsrf(t_index,:) == 1 ) ;
%     if ( length(ncaustic) == 0 )
%         nfold = ncaustic ;
%     else
%         nfold = [ (ncaustic(1)-1) ncaustic(1) ] ;
%     end
    
    % plot wavefronts and rays
    
    plot( wlat(t_index,ndirect), walt(t_index,ndirect), 'b-', ...
          wlat(t_index,ncaustic), walt(t_index,ncaustic), 'g-', ...
          wlat(t_index,nsurface), walt(t_index,nsurface), 'r-', ...
          wlat, walt, 'c-', ...
          xtarget, ytarget, 'k.', ...
          wlat(t_index,ndirect), walt(t_index,ndirect), 'bo', ...
          wlat(t_index,ncaustic), walt(t_index,ncaustic), 'go', ...
          wlat(t_index,nsurface), walt(t_index,nsurface), 'ro' ) ;
    grid; axis( scale ) ;
    xlabel('Range (m)');
    ylabel('Depth (m)');
    title(sprintf( 'pederson deep raytrace t(%d)=%.0f msec', ...
        t_index, 1000*wavefront.travel_time(t_index) ));
    legend('direct','caustic','surface','rays','Location','NorthEast');
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
