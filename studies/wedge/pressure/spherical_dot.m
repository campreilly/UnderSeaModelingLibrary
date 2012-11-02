%%
% Compute dot product in spherical coordinates using the haversine formula.
% 
%   dot = spherical_dot( rho1, theta1, phi1, rho2, theta2, phi2 )
%
%   rho1, theta1, phi1 = spherical coordinates of first vector
%   rho2, theta2, phi2 = spherical coordinates of second vector
%
function dot = spherical_dot( rho1, theta1, phi1, rho2, theta2, phi2 )
    sin_theta = sin( 0.5*(theta1-theta2) ) ;
    sin_phi = sin( 0.5*(phi1-phi2) ) ;
    dot = rho1 .* rho2 .* ( 1.0 - 2.0 ...
        .* ( sin_theta .* sin_theta + sin(theta1) ...
        .* sin(theta2) .* sin_phi .* sin_phi ) ) ;
end