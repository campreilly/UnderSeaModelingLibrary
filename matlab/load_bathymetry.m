%
% load_bathymetry.m - Loads netCDF bathymetry data into Matlab structure
%
% Returns a bottom topology structure with the following fields:
%
%       latitude        bottom latitudes (degrees_north)
%       longitude       bottom longitudes (degrees_east)
%       altitude        bottom altitudes (meters)
% 
% If the file includes a variable for earth_radius, we assume that this
% bathymetry was created by saving a USML data_grid<2> to disk. This format
% stores its data as longitude, co-latitude, and distance from the earth's
% center. This data is automatically converted.

function bathymetry = load_bathymetry(filename)

info = ncinfo(filename);

% search variables for 2D altitude

for n=1:length(info.Variables)
    variable = info.Variables(n);
    if length(variable.Dimensions) == 2
        altitude = ncread(filename, variable.Name) ;
        longitude = ncread(filename, variable.Dimensions(1).Name);
        latitude = ncread(filename, variable.Dimensions(2).Name);
        bathymetry = struct( ...
            'latitude', double(latitude), ...
            'longitude', double(longitude), ...
            'altitude', double(altitude.'));
        break;
    end
end

% check for errors

if ~exist('bathymetry','var')
    error('can not find 2D altitude');
end

% search variables for earth_radius
% if found, treat as output from USML data_grid<2>

for n=1:length(info.Variables)
    variable = info.Variables(n);
    if strcmpi(variable.Name, 'earth_radius')
        earth_radius = ncread(filename,variable.Name);
        bathymetry.longitude = bathymetry.longitude * 180 / pi;
        bathymetry.latitude = 90.0 - bathymetry.latitude * 180 / pi;
        bathymetry.altitude = bathymetry.altitude - earth_radius;
        break;
    end
end
