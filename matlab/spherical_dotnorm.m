%%
% SPHERICAL_DOTNORM Normalized dot product in spherical coordinates
%
% Dot product divided by the magnitude of the two vectors.
% Used to compute the angle between vectors.
% Uses the haversine formula to avoid errors for short distances.
%
%      cos(angle) = dot(v1,v2) / ( |v1| |v2| )
%                 = ( cos(t1)cos(t2) + sin(t1)sin(t2)cos(p1-p2) )
%                 = { 1-2*( sin^2[(t1-t2)/2] + sin(t1)sin(t2)sin^2[(p1-p2)/2] ) }
% 
% The arguments are structures that represent spherical coordinate vectors
% and contain the members rho, theta, and phi.  Vectorized calcuations are 
% supported by using matrices for rho, theta, and phi.  If both arguments
% include matrix members, the calcuations are performed element by element.
%
% syntax:   dotnorm = spherical_dotnorm( v1, v2 )
% inputs:   v1 = structure with rho, theta, and phi
%           v2 = structure with rho, theta, and phi
% outputs:  dotnorm = normalized dot product between these vectors
%
function dotnorm = spherical_dotnorm( v1, v2 )

sint = sin( (v1.theta - v2.theta) / 2 ) ;
sinp = sin( (v1.phi - v2.phi) / 2 ) ;
dotnorm = 1 - 2 * ( sint.*sint ...
    + sin(v1.theta) .* sin(v2.theta) .* sinp .* sinp ) ;

end

