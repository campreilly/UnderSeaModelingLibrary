
clear ; close all
[proploss_x, eigenray_x] = load_netcdf_proploss('eigenray_branch_pt.nc') ;

deg_2_m = 1852.0 * 60.0 / 1e3 ;      % conversion factor from deg lat to km
source_x = proploss_x.source_longitude * deg_2_m ;
source_y = proploss_x.source_latitude * deg_2_m ;
source_z = proploss_x.source_altitude ;
target_x = proploss_x.longitude(3:end) * deg_2_m ;
target_y = proploss_x.latitude(3:end) * deg_2_m ;
target_z = proploss_x.altitude(3:end) ;

figure ;
l = plot3( source_x, source_y, source_z, 'ro', ...
           target_x, target_y, target_z, 'ko' ) ;
grid on ;
set(l(1), 'MarkerFaceColor', 'r', 'MarkerEdgeColor', 'r') ;
set(l(2:end), 'MarkerFaceColor', 'k', 'MarkerEdgeColor', 'k') ;
xlabel('Longitude (km)') ;
ylabel('Latitude (km)') ;
zlabel('Depth (m)') ;
set(gca, 'zlim', [-1500 -500]) ;
set(gca, 'xlim', [-2 2], 'ylim', [-2 2]) ;
