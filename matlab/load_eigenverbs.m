%
% Loads eigenverb data into Matlab structure
%
% The eigenverb structure holds the data for each eigenverb collision with
% a boundary.
%
%       travel_time         path travel time (secs).
%       frequency           frequency (Hz)
%       energy              fraction of total source level (dB)
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

start = [1, 1] ;
count = [inf, inf] ;
energy = ncread(filename,'energy',start,count) ;
leng = ncread(filename,'length',start,count) ;
width = ncread(filename,'width',start,count) ;
start = 1 ;
count = inf ;
travel_time = ncread(filename,'travel_time',start,count) ;
frequency = ncread(filename,'frequency',start,count) ;
latitude = ncread(filename,'latitude',start,count) ;
longitude = ncread(filename,'longitude',start,count) ;
altitude = ncread(filename,'altitude',start,count) ;
direction = ncread(filename,'direction',start,count) ;
grazing = ncread(filename,'grazing_angle',start,count) ;
sound_speed = ncread(filename,'sound_speed',start,count) ;
de_index = ncread(filename,'de_index',start,count) ;
az_index = ncread(filename,'az_index',start,count) ;
source_de = ncread(filename,'source_de',start,count) ;
source_az = ncread(filename,'source_az',start,count) ;
surface = ncread(filename,'surface',start,count) ;
bottom = ncread(filename,'bottom',start,count) ;
caustic = ncread(filename,'caustic',start,count) ;
upper = ncread(filename,'upper',start,count) ;
lower = ncread(filename,'lower',start,count) ;

% translate data into structure

for i=1:length(travel_time)
    eigenverbs(i) = struct( ...
        'travel_time', travel_time(i), ...
        'frequency', frequency, ...
        'energy', energy(:,i), ...
        'length', leng(:,i), ...
        'width', width(:,i), ...
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
