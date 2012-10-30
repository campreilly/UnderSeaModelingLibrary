%%
% Tests vector addition (or subtractio) in spherical coordinates.
%
function spherical_add_test
    close all; clc

    rho1    = sqrt(3) ;
    theta1  = pi/2 ;
    phi1    = pi/4 ;
    
    rho2    = 1 ;
    theta2  = 0 ;
    phi2    = 0 ;
    
    [ rho, theta, phi ] = spherical_add( rho1, theta1, phi1, rho2, theta2, phi2 ) ;
    fprintf('rho = %.1f, theta = %.1f deg, phi = %.1f deg\n',rho, theta*180/pi, phi*180/pi ) ;
    
    [ rho3, theta3, phi3 ] = spherical_add( rho, theta, phi, -rho2, theta2, phi2 ) ;
    fprintf('rho = %.1f, theta = %.1f deg, phi = %.1f deg\n',rho3, theta3*180/pi, phi3*180/pi ) ;
end
