%
% load_netcdf_wavefront.m - Loads wavefront data into Matlab structure
%
% Returns a wavefront structure with the following fields:
%
%       frequency       frequency (Hz)
%       source_de       source D/E angle (degrees)
%       source_az       source AZ angle (degrees_true)
%       travel_time     travel time (seconds)
%       latitude        wavefront latitudes (degrees_north)
%       longitude       wavefront longitudes (degrees_east)
%       altitude        wavefront altitudes (meters)
%       surface         surface reflections (count)
%       bottom          bottom reflections (count)
%       caustic         caustics encounters (count)
%       upper          	upper vertices (count)
%       lower          	lower vertices (count)
%       on_edge         on edge (bool)
%
function wavefront = load_netcdf_wavefront( filename )

    frequency = ncread(filename,'frequency') ;
    source_de = ncread(filename,'source_de') ;
    source_az = ncread(filename,'source_az') ;
    travel_time = ncread(filename,'travel_time') ;
    latitude = ncread(filename,'latitude') ;
    longitude = ncread(filename,'longitude') ;
    altitude = ncread(filename,'altitude') ;
    surface = ncread(filename,'surface') ;
    bottom = ncread(filename,'bottom') ;
    caustic = ncread(filename,'caustic') ;
    upper = ncread(filename,'upper_vertex') ;
    lower = ncread(filename,'lower_vertex') ;
    on_edge = ncread(filename,'on_edge') ;

    wavefront = struct( ...
        'frequency', frequency, ...
        'source_de', source_de, ...
        'source_az', source_az, ...
        'travel_time', travel_time, ...
        'latitude', latitude, ...
        'longitude', longitude, ...
        'altitude', altitude, ...
        'surface', surface, ...
        'bottom', bottom, ...
        'caustic', caustic, ...
        'upper', upper, ...
        'lower', lower, ...
        'on_edge', on_edge ) ;

end