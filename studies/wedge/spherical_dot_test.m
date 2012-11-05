%%
% Tests vector dot product in spherical coordinates.
%
function spherical_dot_test
    close all; clc
    simple_dot_test ;
    great_circle_test ;
end

%%
% Tests the dot product between (0,0,0.5) and (0,2,2).
% The analytic answer is sqrt(2)/2.
%
function simple_dot_test
    rho1    = 0.5 ;         % this is the (0,0,0.5) vector
    theta1  = 0 ;
    phi1    = 0 ;
    
    rho2    = 2 ;           % this is the (0,2,2) vector
    theta2  = pi/4 ;
    phi2    = pi/2 ;
    
    % result should be a dot product of sqrt(2)/2 = 0.7
    dot = spherical_dot( rho1, theta1, phi1, rho2, theta2, phi2 ) ;
    fprintf('dot = %.1f\n',dot ) ;

end

%%
% Computes the great circle distance from LAX to JFK airport using
% the example in Aviation Formulary 1.46 
% http://williams.best.vwh.net/avform.htm#Example.
%
% LAX: 0.592539 north 2.066470 east (radians)
% JFK: 0.709186 north 1.287762 east (radians)
% Distance =  0.623585 radians = 2144 nmi
% Earth Radius = 1852 * (180*60/pi) (meters)
%
function great_circle_test

    earth_radius = 1852 * (180/pi) * 60 ;

    rho1    = earth_radius ;    % LAX
    theta1  = pi/2-0.592539 ;
    phi1    = 2.066470 ;
    
    rho2    = earth_radius ;    % JFK
    theta2  = pi/2-0.709186 ;
    phi2    = 1.287762 ;
    
    % result should be a dot product of one
    
    alpha = acos( spherical_dot( rho1, theta1, phi1, rho2, theta2, phi2 )...
                      / earth_radius^2 ) ;
    distance = earth_radius * alpha ;
    fprintf('distance = %.6f radians = %.0f nmi\n', alpha, distance/1852 ) ;
end
