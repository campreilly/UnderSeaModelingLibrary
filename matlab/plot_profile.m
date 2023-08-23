function plot_profile(ax, profile, latitude, longitude, altitude)
% Visualizer for USML ocean profiles
%   Draws selected profile
% SYNTAX
%   plot_profile(ax, profile, latitude, longitude);
% INPUTS
%   ax = axis on which to plot data
%   profile = ocean profile
%   latitude = latitude to plot
%   longitude = longitude to plot
%
if nargin <= 2 || isempty(latitude)
    latitude = mean(profile.latitude);
end
if nargin <= 3 || isempty(longitude)
    longitude = mean(profile.longitude);
end
if nargin <= 4 || isempty(altitude)
    altitude = [];
end

[~, nlat] = min(abs(latitude-profile.latitude));
[~, nlng] = min(abs(longitude-profile.longitude));
data = squeeze(profile.data(:, nlat, nlng));

plot(ax, data, profile.altitude, 'k', 'LineWidth', 1.5);
if ~isempty(altitude)
    value = interp1(profile.altitude, data, altitude);
    hold(ax, 'on');
    scatter(ax, value, altitude, 30, 'k', 'filled');
    hold(ax, 'off');
end

end
