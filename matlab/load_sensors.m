%
% Loads sensor info cdf
%
function sensors = load_sensors( filename )

% read netCDF data into local variables

sensor_lat = ncread(filename,'sensor_lat') ;
sensor_lng = ncread(filename,'sensor_lng') ;
sensor_alt = ncread(filename,'sensor_alt') ;

num_sensors = length(sensor_lat);


% translate data into array of sensor structs
for n=1:num_sensors
   
    sensors(n).lat = sensor_lat(n);
    sensors(n).lng = sensor_lng(n);
    sensors(n).alt = sensor_alt(n);
    
end

end
