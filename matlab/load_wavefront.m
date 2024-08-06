%
% load_wavefront.m - Loads wavefront data into Matlab structure
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
%       upper           upper vertices (count)
%       lower           lower vertices (count)
%       on_edge         state of on_edge-ness (bool)
%
function wavefront = load_wavefront( filename )

    index = 3:-1:1;
    frequencies = ncread(filename,'frequencies') ;
    source_de = ncread(filename,'source_de');
    source_az = ncread(filename,'source_az');
    travel_time = ncread(filename,'travel_time');
    latitude = permute( ncread(filename,'latitude'), index) ;
    longitude = permute( ncread(filename,'longitude'), index) ;
    altitude = permute( ncread(filename,'altitude'), index) ;
    surface = permute( ncread(filename,'surface'), index) ;
    bottom = permute( ncread(filename,'bottom'), index) ;
    caustic = permute( ncread(filename,'caustic'), index) ;
    upper = permute( ncread(filename,'upper'), index) ;
    lower = permute( ncread(filename,'lower'), index) ;
    on_edge = permute( ncread(filename,'on_edge'), index) ;

wavefront = struct( ...
        'frequencies', frequencies, ...
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
