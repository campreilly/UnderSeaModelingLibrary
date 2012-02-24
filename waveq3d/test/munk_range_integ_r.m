% Compute an analytic solution for the ray paths of the Munk profile in a
% Spherical coordinate system. The Munk profile is an idealized
% representation of a deep sound channel (SOFAR) environment.
%
%       c(r) = c1 [ 1 + p ( z' + exp(-z') - 1 ) r / R
%
%       z' = 2 ( (R-r) - z1 ) / B
%
% where:
%       R   = Earth radius of curvature
%       r   = radial component of position (positive is up)
%       z'  = normalized depth
%       c1  = sound speed on the deep sound channel axis
%       z1  = depth of the deep sound channel axis
%       B   = depth scaling factor
%       p   = perturbation coefficient
%
% A "flat Earth" correction has been applied to the speed profile to
% simplify comparisons to the Cartesian coordinate system.
%
% This test produces an analytic solution for the cycle range as a function
% of launch angle.  For the purpose of this test, the cycle range is
% defined as the range required to complete one period of upward and
% downward refraction.  The cycle range is equal for positive and negative
% launch angles.
%
% The range from the source to the first vertex is computed by integrating
% the equation for dQ/dr using Snell's law for spherical media and the
% quadgk() function.
%
%       r dQ/dr = cot(A) = ac / sqrt( 1 - (ac)^2 )
%
%       a = r cos(Ao) / c0
%
% where
%       Q   = latitude component of position (positive is south)
%       A   = depression/elevation angle at each r (positive is up)
%       A0  = launch angle (positive is up)
%       c0  = sound speed at source depth
%       a   = ray parameter (constant for each launch angle)
%
% To complete the computation of cycle range, distances from upward and
% downward launch angles are combined to compute the complete distance.
% Note that the fact that we are working in Spherical media adds extra
% factors of "r" to both dQ/dr and Snell's law.
%
% Inputs:
%   source_depth    = source depth (meters, positive is down, 
%                     defaults to 1000).
%   source_angles   = list of angles at which to compute cycle range
%                     (degrees, defaults to 0:14 degrees).
%   axis_depth      = depth of the deep sound channel axis
%                     (meters, defaults to 1300).
%   depth_scale     = depth scaling factor (meters, defaults to 1300).
%   axis_speed      = sound speed on the deep sound channel axis
%                     (meters/sec, defaults to 1500).
%   perturb_coeff   = perturbation coefficient
%                     (unitless, defaults to 7.37e-3).
%   earth_radius    = Earth radius of curvature (meters, defaults to
%                     6378101.030201019, the nominal value at 45N).
% Outputs:
%   cycle_ranges   = range required to complete one cycle
%                    as function of source angle (meters)
%
function cycle_ranges = munk_range_integ_r( source_depth, source_angles, ...
    axis_depth, depth_scale, axis_speed, perturb_coeff, earth_radius )

% assign default values used in:
% * F. B. Jensen, W. A. Kuperman, M. B. Porter, and H. Schmidt, 
%   Computational Ocean Acoustics (American Institute of Physics Press, 
%   New York, 1994) pp. 150-153., Figure 3.19 (b).
% * M. B. Porter, "The KRAKEN Normal Mode Program (DRAFT)",
%   Section called "A Deep Water Problem: the Munk Profile", Oct 1997
%   http://oalib.hlsresearch.com/Modes/AcousticsToolbox/manual_html/node8.html

if ( nargin < 1 ), source_depth=1000.0 ; end ;
if ( nargin < 2 ), source_angles=0:14 ; end ;
if ( nargin < 3 ), axis_depth=1300.0 ; end ;
if ( nargin < 4 ), depth_scale=1300.0 ; end ;
if ( nargin < 5 ), axis_speed=1500.0 ; end ;
if ( nargin < 6 ), perturb_coeff=7.37e-3 ; end ;
if ( nargin < 7 ), earth_radius=6378101.030201019 ; end ;

% compute ray parameters

source_angles = source_angles(:)*pi/180.0 ;
launch_angles = unique([ -flipud(source_angles); source_angles ]);
length_angles = length(launch_angles) ;

source_radius = earth_radius - source_depth ;
source_speed = sound_speed(source_radius) ;
ray_param = source_radius * cos(launch_angles) / source_speed ;

% compute vertex range for each source angle

vertex_ranges = zeros(size(launch_angles)) ;
for n = 1:length_angles

    % compute the depth at which ray becomes horizontal

    vertex_speed = 1.0 ./ ray_param(n) ;
    if ( launch_angles(n) > 0.0 )
        limits = earth_radius - [0 axis_depth] ;
    elseif ( launch_angles(n) <= 0.0 )
        limits = earth_radius - [axis_depth 5*axis_depth] ;
    end
    vertex_radius = fzero( @param_diff, limits ) ;
    vertex_depth = earth_radius - vertex_radius ;
    
    % compute the range at which vertex occurs

    if ( launch_angles(n) > 0.0 )
        vertex_ranges(n) = quadgk( @cot_angle, source_radius, vertex_radius ) ;
    elseif ( launch_angles(n) <= 0.0 )
        vertex_ranges(n) = quadgk( @cot_angle, vertex_radius, source_radius ) ;
    end
end

% combine upper and lower parts of the path
% convert to distance units

cycle_ranges = zeros(size(source_angles)) ;
for n = 1:length(source_angles)
    m = abs(launch_angles) == source_angles(n) ;
    cycle_ranges(n) = 2 * sum( vertex_ranges(m) ) * source_radius ;
end

%%
% Compute the Munk profile sound speed at a specific depth.
% Inlcude a "flat Earth" correction to simplify comparisons of results
% to the Cartesian coordinate system.
% 
% W. H. Munk, Sound channel in an exponentially stratified ocean, with
% application to SOFAR, J. Acoust. Soc. Amer. (55) (1974) pp. 220-226.
%
    function c = sound_speed(r)
        zp = 2*((earth_radius-r)-axis_depth)./depth_scale ;
        c = axis_speed * ( 1 + perturb_coeff * ( zp + exp(-zp) - 1 ) ) ...
          .* r / earth_radius ;
%         fprintf( 'd=%.1f c=%.2f\n', (earth_radius-r), c ) ;
    end

%%
% Used to find depth where ray parameter equals that at the vertex
%
    function dc = param_diff(r)
        c = sound_speed(r) ./ r ;
        dc = c - vertex_speed ;
%         fprintf('z=%f c=%f a=%f dc=%f\n',earth_radius-r,c*r,vertex_speed*r,dc*r);
    end

%%
% Snell's law calculation of the cot(angle) at a given depth.
%
    function cot = cot_angle(r)
        cosine = sound_speed(r) / vertex_speed ./ r ;
        cot = cosine ./ sqrt( 1.0 - cosine.*cosine ) ./ r ;
    end
end