%
% Loads propagation loss data into Matlab structure
%
% The proploss structure holds the setup information for the
% scenario and the summed transmission loss data in a structure
% with the following fields.
%
%       source_latitude     source latitudes (degrees_north)
%       source_longitude    source longitudes (degrees_east)
%       source_altitude     target altitudes (meters)
%       launch_de           array of launch D/E angles at source (degrees)
%       launch_az           array of launch D/E angles at source (degrees)
%       time_step           time step used in calculation (secs)
%       frequency           frequency (Hz)
%       latitude            target latitudes (degrees_north)
%       longitude           target longitudes (degrees_east)
%       altitude            target altitudes (meters)
%       intensity           transmission loss (dB)
%       phase               path phase adjustment (rad)
%       travel_time         path travel time (secs).
%       source_de           source D/E angle (degrees)
%       source_az           source AZ angle (degrees_true)
%       target_de           target D/E angle (degrees)
%       target_az           target AZ angle (degrees_true)
%       surface             surface reflections (count)
%       bottom              bottom reflections (count)
%       caustic             caustic encounters (count)
%
% If eigenrays are also requested, they will be returned as an array of
% structures that has the same number of rows and columns as the latitude
% variable.  Each element in this strucute will have an array of values for
% the variables intensity, phase, travel_time,source_de, source_az,
% target_de, target_az, surface, botttom, and caustic.  The elements
% of these array represent a collection of eigenrays for each of the
% targets.
%
function [ proploss, eigenrays ] = load_proploss( filename )

% read netCDF data into local variables

rays = netcdf(filename) ;

source_latitude = rays.VarArray(1).Data ;
source_longitude = rays.VarArray(2).Data ;
source_altitude = rays.VarArray(3).Data ;
launch_de = rays.VarArray(4).Data ;
launch_az = rays.VarArray(5).Data ;
time_step = rays.VarArray(6).Data ;
frequency = rays.VarArray(7).Data ;
latitude = rays.VarArray(8).Data ;
longitude = rays.VarArray(9).Data ;
altitude = rays.VarArray(10).Data ;

proploss_index = 1 + rays.VarArray(11).Data ;
eigenray_index = 1 + rays.VarArray(12).Data ;
eigenray_num = rays.VarArray(13).Data ;

intensity = rays.VarArray(14).Data(proploss_index,:) ;
phase = rays.VarArray(15).Data(proploss_index,:) ;
travel_time = rays.VarArray(16).Data(proploss_index) ;
source_de = rays.VarArray(17).Data(proploss_index) ;
source_az = rays.VarArray(18).Data(proploss_index) ;
target_de = rays.VarArray(19).Data(proploss_index) ;
target_az = rays.VarArray(20).Data(proploss_index) ;
surface = rays.VarArray(21).Data(proploss_index) ;
bottom = rays.VarArray(22).Data(proploss_index) ;
caustic = rays.VarArray(23).Data(proploss_index) ;

% translate scenario and the summed transmission loss data into structure

proploss = struct( ...
    'source_latitude', source_latitude, ...
    'source_longitude', source_longitude, ...
    'source_altitude', source_altitude, ...
    'time_step', time_step, ...
    'frequency', frequency, ...
    'launch_de', launch_de, ...
    'launch_az', launch_az, ...
    'latitude', latitude, ...
    'longitude', longitude, ...
    'altitude', altitude, ...
    'intensity', intensity, ...
    'phase', phase, ...
    'travel_time', travel_time, ...
    'source_de', source_de, ...
    'source_az', source_az, ...
    'target_de', target_de, ...
    'target_az', target_az, ...
    'surface', surface, ...
    'bottom', bottom, ...
    'caustic', caustic ) ;

% translate individual eigenrays into array of structures

if ( nargout >= 2 )
    [N,M] = size( latitude ) ;
    for n=1:N
        for m=1:M
            index = (1:eigenray_num(n,m)) + eigenray_index(n,m) - 1 ;
            intensity = rays.VarArray(14).Data(index,:) ;
            phase = rays.VarArray(15).Data(index,:) ;
            travel_time = rays.VarArray(16).Data(index) ;
            source_de = rays.VarArray(17).Data(index) ;
            source_az = rays.VarArray(18).Data(index) ;
            target_de = rays.VarArray(19).Data(index) ;
            target_az = rays.VarArray(20).Data(index) ;
            surface = rays.VarArray(21).Data(index) ;
            bottom = rays.VarArray(22).Data(index) ;
            caustic = rays.VarArray(23).Data(index) ;

            eigenrays(n,m) = struct( ...
                'intensity', intensity, ...
                'phase', phase, ...
                'travel_time', travel_time, ...
                'source_de', source_de, ...
                'source_az', source_az, ...
                'target_de', target_de, ...
                'target_az', target_az, ...
                'surface', surface, ...
                'bottom', bottom, ...
                'caustic', caustic ) ;
        end
    end
end
