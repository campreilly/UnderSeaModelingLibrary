% Compute analytic solutions for eigenray travel time, source angle, and
% target angle for the deep source in the the Pedersen/Gordon n^2 linear
% profile.  Records analytic eigenray solutions to disk.  
% These values are NaN if the traget is outside of the ray fan.
%
function [direct,caustic] = pedersen_eigenrays_deep( target_ranges )

if ( nargin < 1 ), target_ranges=3000.0:0.25:3100.0 ; end ;

%% compute target range as a function of source angle
% use Snell's Law to match target angles to source angles

source_angles=21:0.1:51.21 ;
x = sqrt(1+2.4/1550*1000)/sqrt(1+2.4/1550*800) ;
target_angles = acos( x * cos(source_angles*pi/180) ) * 180/pi;

source_depth=1000.0;
[Rs,source_speed] = pedersen_range_integ_r( source_depth, source_angles ) ;
Ts = pedersen_time_integ_r( source_depth, source_angles ) ;

target_depth=800.0;
[Rt,target_speed] = pedersen_range_integ_r( target_depth, target_angles ) ;
Tt = pedersen_time_integ_r( target_depth, target_angles ) ;

cycle_ranges = 0.5 * ( Rs + Rt ) ;
cycle_times = 0.5 * ( Ts + Tt ) ;

%% separate path types

[p,n] = max( cycle_ranges ) ;
direct = 1:n ;
caustic = n:length(cycle_ranges) ;

%% compute source angle as a function of target range for direct path

time = interp1(cycle_ranges(direct),cycle_times(direct),target_ranges) ;
sde = interp1(cycle_ranges(direct),source_angles(direct),...
    target_ranges) ;
tde = -acos( target_speed * cos( sde*pi/180) / source_speed ) * 180 / pi ;
direct = struct('range',target_ranges/1e3,'travel_time',time,'source_de',sde,'target_de',tde) ;

%% compute source angle as a function of target range for caustic path

time = interp1(cycle_ranges(caustic),cycle_times(caustic),target_ranges) ;
sde = interp1(cycle_ranges(caustic),source_angles(caustic),...
    target_ranges) ;
tde = -acos( target_speed * cos( sde*pi/180) / source_speed ) * 180 / pi ;
caustic = struct('range',target_ranges/1e3,'travel_time',time,'source_de',sde,'target_de',tde) ;

%% plot the results and save them to disk

figure ;
plot( target_ranges/1e3, direct.source_de, ...
      target_ranges/1e3, caustic.source_de ) ;
grid
set(gca,'Xlim',[target_ranges(1) target_ranges(end)]/1e3) ;
xlabel('Target Range (km)');
ylabel('Source Angle (deg)');
legend('direct','caustic');

figure ;
plot( target_ranges/1e3, direct.target_de, ...
      target_ranges/1e3, caustic.target_de ) ;
grid
set(gca,'Xlim',[target_ranges(1) target_ranges(end)]/1e3) ;
xlabel('Target Range (km)');
ylabel('Target Angle (deg)');
legend('direct','caustic');

figure ;
plot( target_ranges/1e3, direct.travel_time, ...
      target_ranges/1e3, caustic.travel_time ) ;
grid
set(gca,'Xlim',[target_ranges(1) target_ranges(end)]/1e3) ;
xlabel('Target Range (km)');
ylabel('Travel Time (sec)');
legend('direct','surface','Location','Best');

save pedersen_eigenrays_deep direct caustic

end
