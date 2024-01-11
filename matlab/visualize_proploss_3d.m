function visualize_proploss_3d(proploss, bathymetry, frequency, climits, glimits)
% 3D visualizer for USML proploss
%   Creates two figures that can be pasted over each other in PowerPoint.
% SYNTAX
%   visualize_proploss_3d(proploss, bathymetry, frequency, climits, glimits)
% INPUTS
%   proploss        = USML 3D eigneray data loaded from NetCDF file
%   bathymetry      = USML 3D bathymetry data loaded from NetCDF file
%   frequency = frequency to plot
%   climits = color limits on surface plot (optional)
%   glimits = longitude and latitude limits on surface plot (optional)
%
if nargin < 4
    climits = [];
end
if nargin < 5
    glimits = [];
end

% display bathymetry

figure;
plot_bathymetry_3d(gca, bathymetry, glimits);
glimits = axis();

% display proploss as separate plot

figure;
plot_proploss_3d(gca, proploss, frequency, climits, glimits);
colormap(hot);
