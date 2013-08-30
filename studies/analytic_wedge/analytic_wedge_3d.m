%
% Movie of wavefronts for analytic_wedge.
% Illustrates the wavefront as it propogates with a top-down view.
%
% Supports menu controls for next/prev wavefront.
% Maintains user selections for plot axes for next/prev wavefront.
%
clear all ; close all

deg_2_m = 1852.0 * 60.0 / 1e3 ;           % conversion factor from deg lat to km
wavefront = load_wavefront('analytic_wedge_eigenray_wave.nc');
[proploss, eigenrays] = load_proploss('analytic_wedge_proploss.nc') ;
max_time = length( wavefront.travel_time );

% source and target location lat/lon

xtarget = proploss.latitude() * deg_2_m ;
ytarget = proploss.longitude() * deg_2_m ;
ztarget = proploss.altitude() ;

xsource = proploss.source_latitude() * deg_2_m ;
ysource = proploss.source_longitude() * deg_2_m ;
zsource = proploss.source_altitude() ;

% plot wavefront movie

% scale = [ -0.02 0.02 0 0.04 ] ;
f = figure ;
t_index = 5 ;
choice = 0 ;
while ( choice ~= 1 )

    % plot ray trace
    if ~exist('az','var') && ~exist('el','var')
       [az, el] = view(3) ; 
    end
    wlat = double (squeeze(wavefront.latitude(t_index,:,:)) );
    wlat = wlat * deg_2_m ;
    wlon = double (squeeze(wavefront.longitude(t_index,:,:)) );
    wlon = wlon * deg_2_m ;
    walt = double (squeeze(wavefront.altitude(t_index,:,:)) );

    colormap(winter(128)) ;
    h = surface( wlon, wlat, walt ) ;
    hold on ;
    l = plot3( ytarget, xtarget, ztarget, 'ko', ...
           ysource, xsource, zsource, 'mo' ) ;
    grid on ;
    view(az, el) ;
    set(gca, 'YLim', [0 4])
%     set(gca, 'XLim', [-0.25 0.25])
    set(gca, 'ZLim', [-200 0])
    hold off ;
%     axis( scale ) ;
    
    title(sprintf( 'analytic wedge top-down t(%d)=%.1f sec', ...
        t_index, wavefront.travel_time(t_index) ));
    xlabel('Cross-Slope (km)') ;
    ylabel('Range (km)') ;
    zlabel('Depth (m)') ;

    % choose next action
    
    choice = menu('Action','Quit','Next','+10','Previous','-10') ;
    if ( choice == 2 & t_index < max_time ) t_index = t_index + 1 ; end ;
    if ( choice == 3 & t_index < max_time + 10 ) t_index = t_index + 10 ; end ;
    if ( choice == 4 & t_index > 1 ) t_index = t_index - 1 ; end ;
    if ( choice == 5 & t_index < max_time - 10 ) t_index = t_index - 10 ; end ;
%     scale = axis ;      % store user changes to axes
    [az, el] = view() ;
    delete(h) ;

end
delete(f);