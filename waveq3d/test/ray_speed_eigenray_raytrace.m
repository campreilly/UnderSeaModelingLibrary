%
% Movie of wavefronts for analytic_wedge.
% Illustrates the effects of the wedge on a propagated wavefront.
%
% Supports menu controls for next/prev wavefront.
% Maintains user selections for plot axes for next/prev wavefront.
%
clear all ; close all

wavefront = load_wavefront('ray_speed_eigenray_wave.nc') ;
az_index = 12 ;
wlat = squeeze(wavefront.latitude(:,:,az_index)) ;
wlat = (wlat - 36) * ( 1852.0 * 60.0 ) / 1e3 ; % range in km
walt = squeeze(wavefront.altitude(:,:,az_index))  ;
wsrf = squeeze(wavefront.surface(:,:,az_index))  ;
wbtm = squeeze(wavefront.bottom(:,:,az_index))  ;
wcst = squeeze(wavefront.caustic(:,:,az_index))  ;
wedg = squeeze(wavefront.on_edge(:,:,az_index)) ;
max_time = length( wavefront.travel_time ) ;


xsource = 0;
ysource = -10;

% plot wavefront movie

figure ;
scale = [ -90 90 -3800 0 ] ;
t_index = 5 ;
choice = 0 ;
while ( choice ~= 1 )

    % plot ray trace

    lat = wlat(t_index,:) ;
    alt = walt(t_index,:) ;

    plot( wlat, walt, 'c-', ...
          lat, alt, 'r-o', ...
          xsource, ysource, 'ko' ) ;
    grid on ;
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