%%
% SPHERICAL_DOT Dot product in spherical coordinates
%
% Dot product of the two vectors.
% Uses the haversine formula to avoid errors for short distances.
%
%      dot(v1,v2) = |v1| |v2| ( cos(t1)cos(t2) + sin(t1)sin(t2)cos(p1-p2) )
%                 = |v1| |v2| { 1-2*( sin^2[(t1-t2)/2] + sin(t1)sin(t2)sin^2[(p1-p2)/2] ) }
% 
% The arguments are structures that represent spherical coordinate vectors
% and contain the members rho, theta, and phi.  Vectorized calcuations are 
% supported by using matrices for rho, theta, and phi.  If both arguments
% include matrix members, the calcuations are performed element by element.
%
% syntax:   dot = spherical_dot( v1, v2 )
% inputs:   v1 = structure with rho, theta, and phi
%           v2 = structure with rho, theta, and phi
% outputs:  dot = dot product between these vectors
%
function dot = spherical_dot( v1, v2 )

dot = v1.rho .* v2.rho .* spherical_dotnorm( v1, v2 ) ;

end

