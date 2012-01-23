%
% Movie of wavefronts for eigenray_basic.
% Illustrates the effect of extrapolating outside of the wavefront.
%
% Supports menu controls for next/prev wavefront.
% Maintains user selections for plot axes for next/prev wavefront.
%
clear all ; close all

wavefront = load_wavefront('eigenray_basic_wave.nc') ;
az_index = 1 ;
% az_index = 3 ;
wlat = squeeze(wavefront.latitude(:,:,az_index)) ;
wlat = ( wlat - 45 ) * ( 1852.0 * 60.0 ) / 1e3 ; % range in km
walt = squeeze(wavefront.altitude(:,:,az_index))  ;
wsrf = squeeze(wavefront.surface(:,:,az_index))  ;
wbtm = squeeze(wavefront.bottom(:,:,az_index))  ;
wcst = squeeze(wavefront.caustic(:,:,az_index))  ;
max_time = length( wavefront.travel_time ) ;

xtarget  = 0.02 * ( 1852.0 * 60.0 ); % range in meters
ytarget = -1000.0 ;

% plot wavefront movie

figure ;
scale = [ 0 5 -3000 0 ] ;
t_index = 5 ;
choice = 0 ;
while ( choice ~= 1 )

    % plot ray trace

    surf = wsrf(t_index,:) ;
    bott = wbtm(t_index,:) ;
    
    n00 = find( surf == 0 & bott == 0 ) ;
    lat00 = wlat(t_index,n00) ;
    alt00 = walt(t_index,n00) ;
    
    n10 = find( surf == 1 & bott == 0 ) ;
    lat10 = wlat(t_index,n10) ;
    alt10 = walt(t_index,n10) ;

    n01 = find( surf == 0 & bott == 1 ) ;
    lat01 = wlat(t_index,n01) ;
    alt01 = walt(t_index,n01) ;

    n11 = find( surf == 1 & bott == 1 ) ;
    lat11 = wlat(t_index,n11) ;
    alt11 = walt(t_index,n11) ;

    plot( wlat, walt, 'c-', ...
          lat00, alt00, 'r-', lat00, alt00, 'ro', ...
          lat10, alt10, 'b-', lat10, alt10, 'bo', ...
          lat01, alt01, 'g-', lat01, alt01, 'go', ...
          lat11, alt11, 'm-', lat11, alt11, 'mo', ...
          xtarget, ytarget, 'ko' ) ;
    grid on ;
    axis( scale ) ;
    
    title(sprintf( 'eigenray basic t(%d)=%.1f sec', ...
        t_index, wavefront.travel_time(t_index) ));
    xlabel('Range (km)');
    ylabel('Depth (m)');

    % choose next action
    
    choice = menu('Action','Quit','Next','Previous') ;
    if ( choice == 2 & t_index < max_time ) t_index = t_index + 1 ; end ;
    if ( choice == 3 & t_index > 1 ) t_index = t_index - 1 ; end ;
    scale = axis ;      % store user changes to axes

end
