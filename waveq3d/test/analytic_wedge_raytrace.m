%
% Movie of wavefronts for analytic_wedge.
% Illustrates the effects of the wedge on a propagated wavefront.
%
% Supports menu controls for next/prev wavefront.
% Maintains user selections for plot axes for next/prev wavefront.
%
clear all ; close all

wavefront = load_wavefront('analytic_wedge_eigenray_wave.nc') ;
% de_index = 1 ;
az_index = 16 ;
wlat = squeeze(wavefront.latitude(:,:,az_index)) ;
wlat = wlat * ( 1852.0 * 60.0 ) / 1e3 ; % range in km
walt = squeeze(wavefront.altitude(:,:,az_index))  ;
wsrf = squeeze(wavefront.surface(:,:,az_index))  ;
wbtm = squeeze(wavefront.bottom(:,:,az_index))  ;
wcst = squeeze(wavefront.caustic(:,:,az_index))  ;
max_time = length( wavefront.travel_time ) ;

xtarget  = 2.78; % range in meters
ytarget = -30.0 ;

xsource = 0;
ysource = -100;

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
          xsource, ysource, 'ko' ) ;
    grid on ;
    patch([0 4 4],[-200 0 -200],[0.9 0.9 0.9])
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