%
% Loads fathometer data into Matlab structure
%
% The fathometer structure holds the follow information:
%        
%       frequency           frequencies (Hz)
%       source_id           source identifier
%       receiver_id         receiver identifier
%       initial_time        time in sec for first eigenray to return
%       slant_range         range in meters from source to receiver
%       source_latitude     source latitudes (degrees_north)
%       source_longitude    source longitudes (degrees_east)
%       source_altitude     source depth (meters)
%       receiver_latitude   receiver latitudes (degrees_north)
%       receiver_longitude  receiver longitudes (degrees_east)
%       receiver_altitude   receiver depth (meters)
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
%       upper               turn down w/o encounters (count)
%       lower               turn up w/o encounters (count)
%
% If eigenrays are also requested, they will be returned as an array of
% structures that has the same number of rows and columns as the num rays
% by the number of frequencies. Each element in this strucute will have 
% an array of values for the variables intensity, phase, travel_time,
% source_de, source_az, target_de, target_az, surface, botttom, and 
% caustic, upper and lower.
%
function [ fathometer, eigenrays ] = load_fathometer( filename )

% read netCDF file into local variables using matlab's netcdf lib

frequency = ncread(filename,'frequency') ;
source_id = ncread(filename,'source_id') ;
receiver_id = ncread(filename,'receiver_id') ;
initial_time = ncread(filename,'initial_time') ;
slant_range = ncread(filename,'slant_range') ;
source_latitude = ncread(filename,'source_latitude') ;
source_longitude = ncread(filename,'source_longitude') ;
source_altitude = ncread(filename,'source_altitude') ;
receiver_latitude = ncread(filename,'receiver_latitude') ;
receiver_longitude = ncread(filename,'receiver_longitude') ;
receiver_altitude = ncread(filename,'receiver_altitude') ;
intensity = ncread(filename,'intensity') ;
intensity = permute(intensity,[2,1]);
phase = ncread(filename,'phase') ;
phase = permute(phase,[2,1]);
travel_time = ncread(filename,'travel_time') ;
source_de = ncread(filename,'source_de') ;
source_az = ncread(filename,'source_az') ;
target_de = ncread(filename,'target_de') ;
target_az = ncread(filename,'target_az') ;
surface = ncread(filename,'surface') ;
bottom = ncread(filename,'bottom') ;
caustic = ncread(filename,'caustic') ;
upper = ncread(filename,'upper') ;
lower = ncread(filename,'lower') ;

% translate data into output structure

fathometer = struct( ...
    'frequency', frequency, ...
    'source_id', source_id, ...
    'receiver_id', receiver_id, ...
    'initial_time', initial_time, ...
    'slant_range', slant_range, ...
    'source_latitude', source_latitude, ...
    'source_longitude', source_longitude, ...
    'source_altitude', source_altitude, ...
    'receiver_latitude', receiver_latitude, ...
    'receiver_longitude', receiver_longitude, ...
    'receiver_altitude', receiver_altitude, ...
    'intensity', intensity, ...
    'phase', phase, ...
    'travel_time', travel_time, ...
    'source_de', source_de, ...
    'source_az', source_az, ...
    'target_de', target_de, ...
    'target_az', target_az, ...
    'surface', surface, ...
    'bottom', bottom, ...
    'caustic', caustic, ...
    'upper', upper, ...
    'lower', lower ) ;

% translate individual eigenrays into array of structures

if ( nargout >= 2 )
    [N,M] = size (travel_time) ;
    for n=1:N
        e_intensity = intensity(n,:) ;
        e_phase = phase(n,:) ;
        e_travel_time = travel_time(n) ;
        e_source_de = source_de(n) ;
        e_source_az = source_az(n) ;
        e_target_de = target_de(n) ;
        e_target_az = target_az(n) ;
        e_surface = surface(n) ;
        e_bottom = bottom(n) ;
        e_caustic = caustic(n) ;
        e_upper = upper(n) ;
        e_lower = lower(n) ;

        eigenrays(n) = struct( ...
            'intensity', e_intensity, ...
            'phase', e_phase, ...
            'travel_time', e_travel_time, ...
            'source_de', e_source_de, ...
            'source_az', e_source_az, ...
            'target_de', e_target_de, ...
            'target_az', e_target_az, ...
            'surface', e_surface, ...
            'bottom', e_bottom, ...
            'caustic', e_caustic, ...
            'upper', e_upper, ...
            'lower', e_lower ) ;
    end
end
