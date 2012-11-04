%%
% Simpson's rule integration
%
% Integrates a gridded set of points using Simpson's rule. Assumes that the
% function is evaluated at an even number of evenly spaced points.
%
%   result = simpson( h, f )
%
%   h = spacing between ordinate points
%   f = function evaluated at these points
%   
function result = simpson( h, f )

N = length(f) ;
even = 2 * ( 1:(N/2-1) ) ;
odd = 2 * ( 2:(N/2) ) - 1 ;
result = h/3 * ( f(1) + 4*f(even) + 2*f(odd) * f(end) ) ;

end

