%%
% Test Matlab versions of greatCircleDistance range and bearing.
%
function test_great_circle
LAX_to_JFK() ;
JFK_to_LAX() ;
end

%%
% Compute the range and bearing from LAX airport to JFK airport.
% Provided as one of the "worked examples" on the Aviation Formulary V1.46
% website http://williams.best.vwh.net/avform.htm#Example
%
% range = 0.623585 radians = 2144 nmi
% bearing = 1.150035 radians = 66 deg
%
function LAX_to_JFK
    lat1=0.592539; lon1=2.066470;   % LAX location in radians
    lat2=0.709186; lon2=1.287762;   % JFK location in radians
    [range,bearing] = greatCircleDistance( lat1, lon1, lat2, lon2, 1.0 ) ;
    if ( abs(range-0.623585) > 5e-6 )
        error('incorrect range from LAX to JFK');
    end
    if ( abs(bearing-1.150035) > 5e-6 )
        error('incorrect bearing from LAX to JFK');
    end
end

%%
% Compute the range and bearing from JFK airport to LAX airport.
% Tests the ability to reverse bearings.
%
% range = 0.623585 radians = 2144 nmi
% bearing =  4.779726 radians = 274 deg
%
function JFK_to_LAX
    earth_radius = 6371000.0 ;
    lat1=0.709186; lon1=1.287762;   % JFK location in radians
    lat2=0.592539; lon2=2.066470;   % LAX location in radians
    format long
    [range,bearing] = greatCircleDistance( lat1, lon1, lat2, lon2, 1.0 ) ;
    if ( abs(range-0.623585) > 5e-6 )
        error('incorrect range from JFK to LAX');
    end
    if ( abs(bearing- 4.779726) > 5e-6 )
        error('incorrect bearing from JFK to LAX');
    end
end
