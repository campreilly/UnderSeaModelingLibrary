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
function profile = load_profile(filename)

profile = netcdf(filename);

if strcmpi(profile.VarArray(1).Str, 'earth_radius') % USML data_grid<3>
    earth_radius = profile.VarArray(1).Data;
    altitude = profile.VarArray(2).Data - earth_radius;
    latitude = 90 - profile.VarArray(3).Data * 180 / pi;
    longitude = profile.VarArray(4).Data * 180 / pi;
    time = [];
    data = profile.VarArray(5).Data;
else % downloaded from web
    longitude = profile.VarArray(1).Data;
    latitude = profile.VarArray(2).Data;
    altitude = profile.VarArray(3).Data;
    time = profile.VarArray(4).Data;
    data = profile.VarArray(5).Data;
end

profile = struct( ...
    'latitude', double(latitude), ...
    'longitude', double(longitude), ...
    'altitude', double(altitude), ...
    'time', double(time), ...
    'data', double(data));
