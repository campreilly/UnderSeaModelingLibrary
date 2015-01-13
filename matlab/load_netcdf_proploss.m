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
function [ proploss, eigenrays ] = load_netcdf_proploss( filename )

% read netCDF data into local variables

source_latitude = ncread(filename,'source_latitude') ;
source_longitude = ncread(filename,'source_longitude') ;
source_altitude = ncread(filename,'source_altitude') ;
launch_de = ncread(filename,'launch_de') ;
launch_az = ncread(filename,'launch_az') ;
time_step = ncread(filename,'time_step') ;
frequency = ncread(filename,'frequency') ;
latitude = ncread(filename,'latitude') ;
longitude = ncread(filename,'longitude') ;
altitude = ncread(filename,'altitude') ;

proploss_index = 1 + ncread(filename,'proploss_index') ;
eigenray_index = 1 + ncread(filename,'eigenray_index') ;
eigenray_num = ncread(filename,'eigenray_num') ;

% load all values into local workspace
start = [1, 1] ;
count = [inf, inf] ;
intensity = ncread(filename,'intensity',start,count) ;
phase = ncread(filename,'phase',start,count) ;
start = 1 ;
count = inf ;
travel_time = ncread(filename,'travel_time',start,count) ;
source_de = ncread(filename,'source_de',start,count) ;
source_az = ncread(filename,'source_az',start,count) ;
target_de = ncread(filename,'target_de',start,count) ;
target_az = ncread(filename,'target_az',start,count) ;
surface = ncread(filename,'surface',start,count) ;
bottom = ncread(filename,'bottom',start,count) ;
caustic = ncread(filename,'caustic',start,count) ;
% upper = ncread(filename,'upper',start,count) ;
% lower = ncread(filename,'lower',start,count) ;

i = intensity(:,proploss_index) ;
p = phase(:,proploss_index) ;
time = travel_time(proploss_index) ;
sde = source_de(proploss_index) ;
saz = source_az(proploss_index) ;
tde = target_de(proploss_index) ;
taz = target_az(proploss_index) ;
s = surface(proploss_index) ;
b = bottom(proploss_index) ;
c = caustic(proploss_index) ;
% u = upper(:,proploss_index) ;
% l = lower(:,proploss_index) ;

% translate scenario and the summed transmission loss data into structure

proploss = struct( ...
    'source_latitude', source_latitude, ...
    'source_longitude', source_longitude, ...
    'source_altitude', source_altitude, ...
    'time_step', time_step, ...
    'frequency', frequency, ...
    'launch_de', launch_de, ...
    'launch_az', launch_az, ...
    'latitude', latitude', ...
    'longitude', longitude', ...
    'altitude', altitude', ...
    'intensity', i', ...
    'phase', p', ...
    'travel_time', time, ...
    'source_de', sde, ...
    'source_az', saz, ...
    'target_de', tde, ...
    'target_az', taz, ...
    'surface', s, ...
    'bottom', b, ...
    'caustic', c ) ;

% translate individual eigenrays into array of structures

if ( nargout >= 2 )
    [N,M] = size( latitude ) ;
    for n=1:N
        for m=1:M
            index = (1:eigenray_num(n,m)) + eigenray_index(n,m) - 1 ;
            i = intensity(:,index)' ;
            p = phase(:,index)' ;
            time = travel_time(index) ;
            sde = source_de(index) ;
            saz = source_az(index) ;
            tde = target_de(index) ;
            taz = target_az(index) ;
            s = surface(index) ;
            b = bottom(index) ;
            c = caustic(index) ;

            eigenrays(n,m) = struct( ...
                'intensity', i, ...
                'phase', p, ...
                'travel_time', time, ...
                'source_de', sde, ...
                'source_az', saz, ...
                'target_de', tde, ...
                'target_az', taz, ...
                'surface', s, ...
                'bottom', b, ...
                'caustic', c ) ;
        end
    end
end
