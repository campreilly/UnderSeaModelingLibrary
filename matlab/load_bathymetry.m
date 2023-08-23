%
% load_bathymetry.m - Loads netCDF bathymetry data into Matlab structure
%
% Returns a bottom topology structure with the following fields:
%
%       latitude        bottom latitudes (degrees_north)
%       longitude       bottom longitudes (degrees_east)
%       altitude        bottom altitudes (meters)
%
function bathymetry = load_bathymetry(filename)

bottom = netcdf(filename);

if strcmpi(bottom.VarArray(1).Str, 'earth_radius') % USML data_grid<2>
    earth_radius = bottom.VarArray(1).Data;
    latitude = 90 - bottom.VarArray(2).Data * 180 / pi;
    longitude = bottom.VarArray(3).Data * 180 / pi;
    altitude = bottom.VarArray(4).Data - earth_radius;
else % downloaded from web
    latitude = bottom.VarArray(2).Data;
    longitude = bottom.VarArray(1).Data;
    altitude = bottom.VarArray(3).Data;
end

bathymetry = struct( ...
    'latitude', double(latitude), ...
    'longitude', double(longitude), ...
    'altitude', double(altitude));
