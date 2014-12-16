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

wave = netcdf(filename) ;

frequency = wave.VarArray(1).Data ;
source_de = wave.VarArray(2).Data ;
source_az = wave.VarArray(3).Data ;
travel_time = wave.VarArray(4).Data ;
latitude = wave.VarArray(5).Data ;
longitude = wave.VarArray(6).Data ;
altitude = wave.VarArray(7).Data ;
surface = wave.VarArray(8).Data ;
bottom = wave.VarArray(9).Data ;
caustic = wave.VarArray(10).Data ;
upper = wave.VarArray(11).Data ;
lower = wave.VarArray(12).Data ;
on_edge = wave.VarArray(13).Data ;

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
