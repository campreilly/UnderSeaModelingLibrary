% Compare cycle range computed in Cartesian and Spherical coordinates
% using the Munk profile.
%
function munk_range_compute()

source_depth = 1000.0 ;
source_angles = 0:14 ;
cycle_ranges_z = 2*munk_range_integ_z( source_depth, source_angles ) ;
cycle_ranges_r = 2*munk_range_integ_r( source_depth, source_angles ) ;

figure ;
err = cycle_ranges_z-cycle_ranges_r ;
plot( cycle_ranges_r/1e3, err, 'ko-');
grid
xlabel('Range (km)');
ylabel('Range Error (m)');
print -deps munk_range_compute.eps

for n=length(source_angles):-1:2
    fprintf('\t%.6f,\n',cycle_ranges_r(n));
end
for n=1:length(source_angles)
    fprintf('\t%.6f,\n',cycle_ranges_r(n));
end

table = [ cycle_ranges_r/1e3 err ]

end
