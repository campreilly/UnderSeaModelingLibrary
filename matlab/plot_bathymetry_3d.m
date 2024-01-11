function plot_bathymetry_3d(ax, bathymetry, glimits)
% 3D Visualizer for USML Bathymetry
%   Draws bathymetry surface in shades of blue and tan. Land is green. Draws
%   colorbar and labels axes.
% SYNTAX
%   plot_bathymetry(ax, bathymetry)
% INPUTS
%   ax = axis on which to plot data
%   bathymetry = bathymetry data
%   glimits = longitude, latitude, and depth limits on surface plot (optional)
%
if nargin < 3
    glimits = [];
end

x = bathymetry.longitude;
y = bathymetry.latitude;
z = bathymetry.altitude;
z(z > 0) = 0;

% create surface plot

h = surf(ax, x, y, z, 'EdgeColor', 'none', 'FaceColor', 'interp');
zLimits = zlim(ax);
xlabel(ax, 'Longitude (deg)');
ylabel(ax, 'Latitude (deg)');
zlabel(ax, 'Altitude (m)');
axis(ax, 'square');
hbar = colorbar(ax, 'eastoutside');
title(hbar, 'Depth (m)');

% set specialized colormap blue water, tan shallows, green land

map = gray(50);
map(:, 3) = 0.75;
map(end, :) = [0 .5 0];
colormap(ax, map);

% round depths to nearest hundred

zLimits(1) = floor(zLimits(1)/100) * 100;
zLimits(2) = 0;

% set geographic limits

if ~isempty(glimits)
    xlim(glimits(1:2));
    ylim(glimits(3:4));
end
zlim(zLimits);
clim(zLimits);
view(0, 90);

end
