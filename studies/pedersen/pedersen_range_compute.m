% Compare cycle range computed in Cartesian and Spherical coordinates
% using the Pedersen/Gordon n^2 linear profile.
%
function pedersen_range_compute()

source_angles=20:2:50 ;
source_depth=1000.0;
[cycle_ranges_z,theory_ranges_z] = pedersen_range_integ_z( source_depth, source_angles ) ;
fprintf('max range difference from Pedersen paper = %.4e\n',...
    max( abs(cycle_ranges_z-theory_ranges_z) ) ) ;

figure; 
plot( cycle_ranges_z/1e3, cycle_ranges_z-theory_ranges_z, 'ko-' ) ;
grid;
xlabel('Range (km)');
ylabel('Range Error (m)');
print -deps pedersen_range_error_z.eps

[cycle_ranges_r] = pedersen_range_integ_r( source_depth, source_angles ) ;

figure ;
plot( cycle_ranges_r/1e3, cycle_ranges_z-cycle_ranges_r, 'ko-');
grid
xlabel('Range (km)');
ylabel('Range Error (m)');
print -deps pedersen_range_compute.eps
fprintf('cycle ranges:\n');
for n=1:length(source_angles)
    fprintf('\t%.6f,\n',cycle_ranges_r(n));
end

end
