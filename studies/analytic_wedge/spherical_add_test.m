%%
% Tests vector addition (or subtractio) in spherical coordinates.
%
% Test vector addition by adding/subtracting z-hat vector to/from 
% the (1,1,0) vector.  The results should be (1,1,1) for addition
% and (1,1,-1) for subtraction.
%
function spherical_add_test
    close all; clc

    rho1    = sqrt(3) ;     % this is the (1,1,0) vector
    theta1  = pi/2 ;
    phi1    = pi/4 ;
    
    rho2    = 1 ;           % this is the z-hat vector
    theta2  = 0 ;
    phi2    = 0 ;
    
    % adds z-hat to (1,1,0) vector.
    % answer should be rho = 2.0, theta = 60.0 deg, phi = 45.0 deg
    % which is the same as the (1,1,1) vector
    
    [ rho, theta, phi ] = spherical_add( rho1, theta1, phi1, rho2, theta2, phi2 ) ;
    fprintf('rho = %.1f, theta = %.1f deg, phi = %.1f deg\n',rho, theta*180/pi, phi*180/pi ) ;
    
    % subtracts z-hat from (1,1,0) vector.
    % answer should be rho = 1.7, theta = 90.0 deg, phi = 45.0 deg
    % which is the same as the (1,1,-1) vector
    
    [ rho3, theta3, phi3 ] = spherical_add( rho, theta, phi, -rho2, theta2, phi2 ) ;
    fprintf('rho = %.1f, theta = %.1f deg, phi = %.1f deg\n',rho3, theta3*180/pi, phi3*180/pi ) ;
end
