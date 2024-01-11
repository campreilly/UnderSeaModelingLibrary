function [hrays, geo_line] = plot_raytrace_2d(ax, wavefront, azSelected, deList)
% 2D Visualizer for USML ray paths
%   Draws select ray paths.
% SYNTAX
%   plot_raytrace_2d(ax, wavefront, azList, deList)
% INPUTS
%   ax = axis on which to plot data
%   wavefront = ray path data
%   azList = list of azimuths angles to plot
%   deList = list of depression/elevation angles to plot
%
if nargin <= 2 || isempty(azSelected)
    azSelected = 0;
end
if nargin <= 3 || isempty(deList)
    deList = wavefront.source_de;
end

% shift from (time,de,az) to (time,index) format

N = length(wavefront.travel_time);
latitude = reshape(wavefront.latitude, N, []);
longitude = reshape(wavefront.longitude, N, []);
altitude = reshape(wavefront.altitude, N, []);

% find DE and AZ for each index in data

[source_az, source_de] = meshgrid(wavefront.source_az', wavefront.source_de');
source_de = source_de(:);
source_az = source_az(:);
n = ismember(source_az, azSelected) & ismember(source_de, deList);

% compute range to each latitude and longitude

earth_radius = 6371000.0;
v1.theta = deg2rad(90-latitude(1, 1));
v1.phi = deg2rad(longitude(1, 1));
v1.rho = earth_radius;
v2.theta = deg2rad(90-latitude(:, n));
v2.phi = deg2rad(longitude(:, n));
v2.rho = earth_radius * ones(size(v2.phi));
range = spherical_dist(v1, v2);

hrays = plot(ax, range/1e3, altitude(:, n), 'LineWidth', 1.5);

% use greate circle calcs to compute line over which bathymetry will be
% plotted

d = max(range(:)) / earth_radius;
lat1 = latitude(1, 1);
lng1 = longitude(1, 1);
lat2 = asind(sind(lat1)*cos(d)+cosd(lat1)*sin(d)*cosd(azSelected));
if cosd(lat1) == 0
    lng2 = lng1;
else
    lng2 = lng1 + asind(sind(azSelected)*sin(d)/cosd(lat2));
end
geo_line(1) = lng1;
geo_line(2) = lng2;
geo_line(3) = lat1;
geo_line(4) = lat2;

end
