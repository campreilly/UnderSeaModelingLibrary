% Compute analytic solutions for eigenray travel time, source angle, and
% target angle for the shallow source in the the Pedersen/Gordon n^2 linear
% profile.  Records analytic eigenray solutions to disk.  
% These values are NaN if the traget is outside of the ray fan.
%
function [direct,surface] = pedersen_eigenrays_shallow( target_ranges )

if ( nargin < 1 ), target_ranges=500.0:1.0:1000.0 ; end ;

%% compute target range as a function of source angle

source_angles=0.1:0.1:25 ;
source_depth=75.0;
cycle_ranges = pedersen_range_integ_r( source_depth, source_angles ) ;
cycle_times = pedersen_time_integ_r( source_depth, source_angles ) ;

%% separate path types

[p,n] = max( cycle_ranges ) ;
direct = 1:n ;
surface = n:length(cycle_ranges) ;

%% compute times and angles as a function of target range for direct path

travel_time = interp1(cycle_ranges(direct),cycle_times(direct),target_ranges) ;
source_de = interp1(cycle_ranges(direct),source_angles(direct),target_ranges) ;
target_de = -source_de ;
direct = struct('range',target_ranges/1e3,'travel_time',travel_time,'source_de',source_de,'target_de',target_de) ;

%% compute times and angles as a function of target range for surface reflected path

travel_time = interp1(cycle_ranges(surface),cycle_times(surface),target_ranges) ;
source_de = interp1(cycle_ranges(surface),source_angles(surface),target_ranges) ;
target_de = -source_de ;
surface = struct('range',target_ranges/1e3,'travel_time',travel_time,'source_de',source_de,'target_de',target_de) ;

%% plot the results andsavee them to disk

figure ;
plot( direct.range, direct.source_de, ...
      surface.range, surface.source_de ) ;
grid
set(gca,'Xlim',[target_ranges(1) target_ranges(end)]/1e3) ;
xlabel('Target Range (km)');
ylabel('Source Angle (deg)');
legend('direct','surface');

figure ;
plot( direct.range, direct.target_de, ...
      surface.range, surface.target_de ) ;
grid
set(gca,'Xlim',[target_ranges(1) target_ranges(end)]/1e3) ;
xlabel('Target Range (km)');
ylabel('Target Angle (deg)');
legend('direct','surface');

figure ;
plot( direct.range, direct.travel_time, ...
      surface.range, surface.travel_time ) ;
grid
set(gca,'Xlim',[target_ranges(1) target_ranges(end)]/1e3) ;
xlabel('Target Range (km)');
ylabel('Travel Time (sec)');
legend('direct','surface');

save pedersen_eigenrays_shallow direct surface

end
