function d = greatCircleDistance(phi_s, lambda_s, phi_f, lambda_f, r)
% compute the great circle distance given lat and long for two points
% optionally, a fifth parameter (r) can be specified. If this paramter
% isn't specified it's assumed to be the mean radius of the earth. The
% calculation is done using the Vincenty formula.
% Note: That the Vincenty equation requires the latitude and longitude 
%       to be specified in radians. This function will accept either 
%       radians, DMS or DM. If the input angles are scalar, 
%       they are assumed to be radians. If they are structures or 
%       they are not scalars, then they are assumed to be either 
%       DMS or DM and are converted to radians as needed.
%
% INPUTS:
% phi_s    = latitude of the standpoint (base) [rad]
% lambda_s = longitude of the standpoint (base) [rad]
% phi_f    = latitude of the forepoint (destination) [rad]
% lambda_f = longitude of the forepoint (destination) [rad]
% r        = radius of the sphere [units determine units of d]
%
% OUTPUT:
% d        = great circle distance from standpoint to forepoint
%
% See http://en.wikipedia.org/wiki/Great-circle_distance

% If no arguments, bail out
if nargin < 4
    fprintf('Usage: greatCircleDistance(phi_s, lambda_s, phi_f, lambda_f, r)\n')
    return
end

% If no radius supplied, assume the mean radius of the earth in km
if nargin < 5
    r = 6371.01; % km
end

% convert from degrees minutes seconds to radians as needed
if isstruct(phi_s) || (length(phi_s) > 1 && ~isstruct(phi_s))
    phi_s = dms2r(phi_s);
end
if isstruct(lambda_s) || (length(lambda_s) > 1 && ~isstruct(lambda_s))
    lambda_s = dms2r(lambda_s);
end
if isstruct(phi_f) || (length(phi_f) > 1 && ~isstruct(phi_f))
    phi_f = dms2r(phi_f);
end
if isstruct(lambda_f) || (length(lambda_f) > 1 && ~isstruct(lambda_f))
    lambda_f = dms2r(lambda_f);
end

% Compute Delta lambda (delta longitude)
Delta_lambda = lambda_f - lambda_s;

% Compute Delta sigma (central angle)
Delta_sigma = atan2(sqrt((cos(phi_f)*sin(Delta_lambda))^2 + (cos(phi_s)*sin(phi_f) - sin(phi_s)*cos(phi_f)*cos(Delta_lambda))^2), ...
    sin(phi_s)*sin(phi_f) + cos(phi_s)*cos(phi_f)*cos(Delta_lambda));

d = r*Delta_sigma;

function r = dms2r(dms)

if isstruct(dms)
    r = sign(dms.deg)*(abs(dms.deg) + (dms.min + dms.sec/60)/60)*pi/180;
elseif length(dms) == 3
    r = sign(dms(1))*(abs(dms(1)) + (dms(2) + dms(3)/60)/60)*pi/180;
elseif length(dms) == 2
    r = sign(dms(1))*(abs(dms(1)) + dms(2)/60)*pi/180;
else
    r = nan;
end