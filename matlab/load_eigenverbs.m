%
% Loads eigenverb data into Matlab structure
%
% The eigenverb structure holds the data for each eigenverb collision with
% a boundary.
%
%       travel_time         path travel time (secs).
%       frequency           frequency (Hz)
%       power               fraction of total source level (dB)
%       length              length of interface projection in D/E (meters)
%       width               length of interface projection in D/E (meters)
%       latitude            latitude of boundary interaction (degrees_north)
%       longitude           longitude of bounary interaction (degrees_east)
%       altitude            altitude of the bounary (meters)
%       direction           compass heading for the "length" axis (degrees)
%       grazing             grazing angle for the boundary (degrees)
%       sound_speed         speed of sound at interface (m/s)
%       de_index            index for the de launch angle (index)
%       az_index            index for the az launch angle (index)
%       source_de           launch D/E angle at source (degrees)
%       source_az           launch AZ angle at source (degrees)
%       surface             surface reflections (count)
%       bottom              bottom reflections (count)
%       caustic             caustic reflections (count)
%       upper               bottom reflections (count)
%       lower               surface reflections (count)
%
function eigenverbs = load_eigenverbs( filename )

% read netCDF data into local variables

power = ncread(filename,'power') ;
leng = ncread(filename,'length') ;
width = ncread(filename,'width') ;
travel_time = ncread(filename,'travel_time') ;
frequency = ncread(filename,'frequency') ;
latitude = ncread(filename,'latitude') ;
longitude = ncread(filename,'longitude') ;
altitude = ncread(filename,'altitude') ;
direction = ncread(filename,'direction') ;
grazing = ncread(filename,'grazing_angle') ;
sound_speed = ncread(filename,'sound_speed') ;
de_index = ncread(filename,'de_index') ;
az_index = ncread(filename,'az_index') ;
source_de = ncread(filename,'source_de') ;
source_az = ncread(filename,'source_az') ;
surface = ncread(filename,'surface') ;
bottom = ncread(filename,'bottom') ;
caustic = ncread(filename,'caustic') ;
upper = ncread(filename,'upper') ;
lower = ncread(filename,'lower') ;

% translate data into structure

for i=1:length(travel_time)
    eigenverbs(i) = struct( ...
        'travel_time', travel_time(i), ...
        'frequency', frequency, ...
        'power', power(:,i), ...
        'length', leng(i), ...
        'width', width(i), ...
        'latitude', latitude(i), ...
        'longitude', longitude(i), ...
        'altitude', altitude(i), ...
        'direction', direction(i), ...
        'grazing', grazing(i), ...
        'sound_speed', sound_speed(i), ...
        'de_index', de_index(i), ...
        'az_index', az_index(i), ...
        'source_de', source_de(i), ...
        'source_az', source_az(i), ...
        'surface', surface(i), ...
        'bottom', bottom(i), ...
        'caustic', caustic(i), ...
        'upper', upper(i), ...
        'lower', lower(i) ) ;
end

end
