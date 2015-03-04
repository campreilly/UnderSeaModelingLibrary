%%
% SPHERICAL_DIST2 Squared distance between vectors in spherical coordinates
%
% Square of the distance between two vectors using the dot product
% form of Pythagoras's theorem.
%
%      dist2 = |v1|^2 + |v2|^2 - 2 dot(v1,v2)
% 
% The arguments are structures that represent spherical coordinate vectors
% and contain the members rho, theta, and phi.  Vectorized calcuations are 
% supported by using matrices for rho, theta, and phi.  If both arguments
% include matrix members, the calcuations are performed element by element.
%
% syntax:   dist2 = spherical_dist2( v1, v2 )
% inputs:   v1 = structure with rho, theta, and phi
%           v2 = structure with rho, theta, and phi
% outputs:  dist2 = square of the distance between these vectors
%
function dist2 = spherical_dist2( v1, v2 )

dist2 = v1.rho .* v1.rho + v2.rho .* v2.rho - 2 * spherical_dot(v1,v2);

end

