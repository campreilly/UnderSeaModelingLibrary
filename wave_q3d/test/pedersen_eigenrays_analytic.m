% Compare cycle range computed in Cartesian and Spherical coordinates
% using the Pedersen/Gordon n^2 linear profile.
%
% function pedersen_eigenrays_analytic()

target_ranges = 3000:0.1:3100 ;
source_angles=0.1:0.1:25 ;
source_depth=75.0;
cycle_ranges_r = pedersen_range_integ_r( source_depth, source_angles ) ;

[p,n] = max( cycle_ranges_r ) ;
direct = 1:n ;
caustic = n:length(cycle_ranges_r) ;

figure ;
plot( source_angles(direct), cycle_ranges_r(direct)/1e3, ...
      source_angles(caustic), cycle_ranges_r(caustic)/1e3 ) ;
grid
[p,n] = max( cycle_ranges_r ) ;
direct = 1:n ;
caustic = n:length(cycle_ranges_r) ;

figure ;
plot( source_angles(direct), cycle_ranges_r(direct)/1e3, ...
      source_angles(caustic), cycle_ranges_r(caustic)/1e3 ) ;
grid
xlabel('Source Angle (deg)');
ylabel('Cycle Range (km)');
legend('direct','caustic');
% end
