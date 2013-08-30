%
% Movie of wavefronts for analytic_wedge.
% Illustrates the effects of the wedge on a propagated wavefront.
%
% Supports menu controls for next/prev wavefront.
% Maintains user selections for plot axes for next/prev wavefront.
%
% clear all ; close all
deg_2_m = 1852.0 * 60.0 / 1e3 ;           % conversion factor from deg lat to km

wavefront = load_wavefront('analytic_wedge_eigenray_wave.nc') ;
[proploss, eigenrays] = load_proploss('analytic_wedge_proploss.nc') ;
% de_index = 1 ;
az_index = 91 ;
wlat = squeeze(wavefront.latitude(:,:,az_index)) ;
wlat = wlat * deg_2_m ; % range in km
walt = squeeze(wavefront.altitude(:,:,az_index))  ;
max_time = length( wavefront.travel_time ) ;

xtarget  = proploss.latitude() * deg_2_m ;
ytarget = proploss.altitude() ;

xsource = proploss.source_latitude() * deg_2_m ;
ysource = proploss.source_altitude() ;

% plot wavefront movie

figure ;
scale = [ 0 4 -200 0 ] ;
t_index = 5 ;
choice = 0 ;
while ( choice ~= 1 )

    % plot ray trace

    lat = wlat(t_index,:) ;
    alt = walt(t_index,:) ;

    plot( wlat, walt, 'c-', ...
          lat, alt, 'r-o', ...
          xtarget, ytarget, 'ko', ...
          xsource, ysource, 'bo' ) ;
    grid on ;
%     patch([0 4 4],[-200 0 -200],[0.9 0.9 0.9])
    axis( scale ) ;
    
    title(sprintf( 'analytic wedge t(%d)=%.1f sec', ...
        t_index, wavefront.travel_time(t_index) ));
    xlabel('Range (km)');
    ylabel('Depth (m)');

    % choose next action
    
    choice = menu('Action','Quit','Next','+10','Previous','-10') ;
    if ( choice == 2 & t_index < max_time ) t_index = t_index + 1 ; end ;
    if ( choice == 3 & t_index < max_time + 10 ) t_index = t_index + 10 ; end ;
    if ( choice == 4 & t_index > 1 ) t_index = t_index - 1 ; end ;
    if ( choice == 5 & t_index < max_time - 10 ) t_index = t_index - 10 ; end ;
    scale = axis ;      % store user changes to axes

end