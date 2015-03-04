%%
% SPHERICAL_DIST Distance between vectors in spherical coordinates
%
% The distance between two vectors using the dot product
% form of Pythagoras's theorem.
%
%      dist = sqrt( |v1|^2 + |v2|^2 - 2 dot(v1,v2) )
% 
% The arguments are structures that represent spherical coordinate vectors
% and contain the members rho, theta, and phi.  Vectorized calcuations are 
% supported by using matrices for rho, theta, and phi.  If both arguments
% include matrix members, the calcuations are performed element by element.
%
% syntax:   dotnorm = spherical_dotnorm( v1, v2 )
% inputs:   v1 = structure with rho, theta, and phi
%           v2 = structure with rho, theta, and phi
% outputs:  dist = distance between these vectors
%
function dist = spherical_dist( v1, v2 )

dist = sqrt( spherical_dist2( v1, v2 ) ) ;

end

