%
% Movie of wavefronts for analytic_wedge.
% Illustrates the wavefront as it propogates with a top-down view.
%
% Supports menu controls for next/prev wavefront.
% Maintains user selections for plot axes for next/prev wavefront.
%
clear all ; close all

wavefront = load_wavefront('analytic_wedge_eigenray_wave.nc');
wlat = squeeze(wavefront.latitude(:,125,:));
wlon = squeeze(wavefront.longitude(:,125,:));
max_time = length( wavefront.travel_time );

% source and target location lat/lon

xtarget = 0;
ytarget = 0.025;

xsource = 0;
ysource = 0;

% plot wavefront movie

figure ;
scale = [ -0.02 0.02 0 0.04 ] ;
t_index = 5 ;
choice = 0 ;
while ( choice ~= 1 )

    % plot ray trace

    lat = wlat(t_index,:,:) ;
    lon = wlon(t_index,:,:) ;

    plot( wlon, wlat, 'b-', ...
          lon, lat, 'r-o', ...
          xtarget, ytarget, 'ko', ...
          xsource, ysource, 'ko' ) ;
    grid on ;
    axis( scale ) ;
    
    title(sprintf( 'analytic wedge top-down t(%d)=%.1f sec', ...
        t_index, wavefront.travel_time(t_index) ));
    xlabel('longitutde');
    ylabel('latitude');

    % choose next action
    
    choice = menu('Action','Quit','Next','+10','Previous','-10') ;
    if ( choice == 2 & t_index < max_time ) t_index = t_index + 1 ; end ;
    if ( choice == 3 & t_index < max_time + 10 ) t_index = t_index + 10 ; end ;
    if ( choice == 4 & t_index > 1 ) t_index = t_index - 1 ; end ;
    if ( choice == 5 & t_index < max_time - 10 ) t_index = t_index - 10 ; end ;
    scale = axis ;      % store user changes to axes

end