%
% load_profile.m - Loads netCDF profile data into Matlab structure
%
% Returns a profile structure with the following fields:
%
%       latitude        profile latitudes (degrees_north)
%       longitude       profile longitudes (degrees_east)
%       altitude        profile altitudes (meters)
%       time            profile day of the year (days)
%       data            data for each point in profile
%                       1st dminension is time
%                       2nd dminension is depth
%                       3rd dminension is latitude
%                       4th dminension is longitude
%
% If the file includes a variable for earth_radius, we assume that this
% profile was created by saving a USML data_grid<3> to disk. This format
% stores its data as longitude, co-latitude, and distance from the earth's
% center. This data is automatically converted.

function profile = load_profile(filename)

info = ncinfo(filename);
index = 3:-1:1;

% search variables for 2D altitude

for n=1:length(info.Variables)
    variable = info.Variables(n);
    if length(variable.Dimensions) >= 3
        data = ncread(filename, variable.Name) ;
        longitude = ncread(filename, variable.Dimensions(1).Name);
        latitude = ncread(filename, variable.Dimensions(2).Name);
        altitude = ncread(filename, variable.Dimensions(3).Name);
        profile = struct( ...
            'latitude', double(latitude), ...
            'longitude', double(longitude), ...
            'altitude', double(-altitude), ...
            'data', double( permute(data, index) ));
        break;
    end
end

% check for errors

if ~exist('profile','var')
    error('can not find 2D altitude');
end

% search variables for earth_radius
% if found, treat as output from USML data_grid<3>

for n=1:length(info.Variables)
    variable = info.Variables(n);
    if strcmpi(variable.Name, 'earth_radius')
        earth_radius = ncread(filename,variable.Name);
        profile.longitude = profile.longitude * 180 / pi;
        profile.latitude = 90.0 - profile.latitude * 180 / pi;
        profile.altitude = -profile.altitude - earth_radius;
        break;
    end
end
