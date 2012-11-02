%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%  test program for the cartesian distance program
%
function cart_distance_test
close all;

x_source = 4000;
z_source = 100;

x_target = 2000;
z_target = 30;

distance = cart_distance(x_source,atan(z_source/x_source),x_target,z_target);

fprintf ("The seperation distance for\nr_source = %1.0f xhat + %1.0f zhat\nr_target = %1.0f xhat + %1.0f zhat\nwith values in meters, is: %1.5f\n", x_source, z_source, x_target, z_target, distance);

end
