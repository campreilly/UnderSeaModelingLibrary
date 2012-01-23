%
% load_bathymetry.m - Loads netCDF bathymetry data into Matlab structure
%
% Returns a bottom topology structure with the following fields:
%
%       latitude        bottom latitudes (degrees_north)
%       longitude       bottom longitudes (degrees_east)
%       altitude        bottom altitudes (meters)
%
function bathymetry = load_bathymetry( filename )

bottom = netcdf(filename) ;

latitude = bottom.VarArray(2).Data ;
longitude = bottom.VarArray(1).Data ;
altitude = bottom.VarArray(3).Data ;

bathymetry = struct( ...
    'latitude', double(latitude), ...
    'longitude', double(longitude), ...
    'altitude', double(altitude) ) ;
