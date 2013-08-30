%%
% Vector addition (or subtraction) in spherical coordinates.
% 
%   [ rho, theta, phi ] = spherical_add( rho1, theta1, phi1, rho2, theta2, phi2 )
%
%   rho, theta, phi    = spherical coordinates of vector sum
%   rho1, theta1, phi1 = spherical coordinates of first vector
%   rho2, theta2, phi2 = spherical coordinates of second vector
%
function [ rho, theta, phi ] = spherical_add( rho1, theta1, phi1, rho2, theta2, phi2 )

    % convert to Cartesian coordinates
    
    sin_theta1 = sin( theta1 ) ;
    cos_theta1 = cos( theta1 ) ;
    sin_phi1 = sin( phi1 ) ;
    cos_phi1 = cos( phi1 ) ;
    
    sin_theta2 = sin( theta2 ) ;
    cos_theta2 = cos( theta2 ) ;
    sin_phi2 = sin( phi2 ) ;
    cos_phi2 = cos( phi2 ) ;
    
    % compute sum in Cartesian coordinates
    
    x = rho1 .* sin_theta1 .* cos_phi1 + rho2 .* sin_theta2 .* cos_phi2 ;
    y = rho1 .* sin_theta1 .* sin_phi1 + rho2 .* sin_theta2 .* sin_phi2 ;
    z = rho1 .* cos_theta1 + rho2 .* cos_theta2 ;
    
    % convert result to Spherical coodinates
    
    rsquared = x.*x + y.*y ;
    rho = sqrt( rsquared + z.*z ) ;
    r = sqrt( rsquared ) ;
    phi = atan2( x, y ) ;
    theta = atan2( r, z ) ;
end
