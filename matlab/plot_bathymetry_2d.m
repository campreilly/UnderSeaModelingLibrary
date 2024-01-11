function hbathy = plot_bathymetry_2d(ax, bathymetry, geo_line)
% 3D Visualizer for USML Bathymetry
%   Draws bathymetry surface in shades of blue and tan. Land is green. Draws
%   colorbar and labels axes.
% SYNTAX
%   plot_bathymetry_2d(ax, bathymetry)
% INPUTS
%   ax = axis on which to plot data
%   bathymetry = bathymetry data
%   geo = geographic line over which to compute bathymetry
%
x = bathymetry.longitude;
y = bathymetry.latitude;
z = bathymetry.altitude;
z(z > 0) = 0;
longitude = geo_line(1) + (geo_line(2) - geo_line(1)) * (0:1e-3:1);
latitude = geo_line(3) + (geo_line(4) - geo_line(3)) * (0:1e-3:1);
depth = interp2(x, y, z, longitude, latitude);

earth_radius = 6371000.0;

v1.theta = deg2rad(90-latitude(1));
v1.phi = deg2rad(longitude(1));
v1.rho = earth_radius;
v2.theta = deg2rad(90-latitude);
v2.phi = deg2rad(longitude);
v2.rho = earth_radius * ones(size(v2.phi));
range = spherical_dist(v1, v2);

% create surface plot

hbathy = plot(ax, range/1e3, depth, 'k-', 'LineWidth', 1.5);

end
