function hrays = plot_raytrace_3d(ax, wavefront, azList, deList)
% 3D Visualizer for USML ray paths
%   Draws select ray paths.
% SYNTAX
%   plot_wavefront(ax, wavefront, azList, deList)
% INPUTS
%   ax = axis on which to plot data
%   wavefront = ray path data
%   azList = list of azimuths angles to plot
%   deList = list of depression/elevation angles to plot
%
if nargin <= 2 || isempty(azList)
    azList = wavefront.source_az;
end
if nargin <= 3 || isempty(deList)
    deList = wavefront.source_de;
end

N = length(wavefront.travel_time);
latitude = reshape(wavefront.latitude, N, []);
longitude = reshape(wavefront.longitude, N, []);
altitude = reshape(wavefront.altitude, N, []);

[source_az, source_de] = meshgrid(wavefront.source_az', wavefront.source_de');
source_de = source_de(:);
source_az = source_az(:);

n = ismember(source_az, azList) & ismember(source_de, deList);
hrays = plot3(ax, longitude(:, n), latitude(:, n), altitude(:, n), ...
    'k', 'LineWidth', 1.5);

end
