function plot_proploss_3d(ax, proploss, frequency, climits, glimits)
% 3D Visualizer for USML Propagation Loss
%   Draws propagation loss on a latitude and longitude surface. If the
%   proploss includes a field for signal_excess, it plots that one instead.
% SYNTAX
%   plot_proploss_3d(ax, proploss, frequency, limits)
% INPUTS
%   ax = axis on which to plot data
%   proploss = USML proploss data
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

% select data to plot

if isfield(proploss,'signal_excess')
    data = proploss.signal_excess;
    label = 'SE (dB)';
else
    data = -proploss.intensity;
    label = 'TL (dB)';
end

% compute range to each latitude and longitude

latitude = proploss.latitude;
longitude = proploss.longitude;
altitude = zeros(size(longitude));
[N, M] = size(longitude);

[~, index] = min(abs(proploss.frequency-frequency));
intensity = data(:, index);
intensity = reshape(intensity, N, M);
if ~isempty(climits)
    intensity(intensity < climits(1)) = NaN;
    intensity(intensity > climits(2)) = climits(2);
end

surf(longitude, latitude, altitude, intensity, ...
    'EdgeColor', 'none', 'FaceColor', 'interp');
view(ax, 0, 90);
xlabel(ax, 'Longitude (deg)');
ylabel(ax, 'Latitude (deg)');
axis(ax, 'square');
h = colorbar(ax, 'eastoutside');
title(h, label);

% set geographic limits

limits = axis();
if ~isempty(glimits)
    limits(1:4) = glimits(1:4);
end
limits(5) = -1;
limits(6) = 0;
axis(limits);
view(0, 90);

end
