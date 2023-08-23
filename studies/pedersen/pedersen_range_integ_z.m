% Compute an analytic solution for the ray paths of the Pedersen profile in
% a Spherical coordinate system. The Pedersen profile is an extreme test
% case for a downward refracting n^2 linear profile.
%
%       c(r) = [ c0 / sqrt( 1 + 2 g0 (R-r) / c0 ) ] r / R
%
% where:
%       R   = Earth radius of curvature
%       r   = radial component of position (positive is up)
%       c0  = sound speed at the ocean surface
%       g0  = sound speed gradient at at the ocean surface
%
% A "flat Earth" correction has been applied to the speed profile to
% simplify comparisons to the Cartesian coordinate system.
%
% This test produces an analytic solution for the cycle range as a function
% of launch angle.  For the purpose of this test, the cycle range is
% defined as the range required to reach the upward refracting apex and
% return to the original depth. For a source at 1000 yds, only angles in
% the range (0,50.44] degrees (up) will produce a purely refractive cycle
% range.  Following Pedersen example, we will focus our analysis on angles
% in the range [20,50].
%
% The range from the source to the first vertex is computed by integrating
% the equation for dQ/dr using Snell's law for spherical media and the
% quadgk() function.
%
%       r dQ/dr = cot(A) = ac / sqrt( 1 - (ac)^2 )
%
%       a = r cos(A1) / c1
%
% where
%       Q   = latitude component of position (positive is south)
%       A   = depression/elevation angle at each r (positive is up)
%       A1  = launch angle (positive is up)
%       c1  = sound speed at source depth
%       a   = ray parameter (constant for each launch angle)
%
% To complete the computation of cycle range, distances from upward and
% downward launch angles are combined to compute the complete distance.
% Note that the fact that we are working in Spherical media adds extra
% factors of "r" to both dQ/dr and Snell's law.
%
% Inputs:
%   source_depth    = source depth (meters, positive is down, 
%                     defaults to 911.4 m).
%   source_angles   = list of angles at which to compute cycle range
%                     (degrees, must be > 0, defaults to 20:2:50 degrees).
%   surface_speed   = sound speed at the ocean surface
%                     (meters/sec, defaults to 1550.0).
%   surface_grad    = sound speed gradient at at the ocean surface,
%                     positive for downward refracting environments.
%                     (1/s, defaults to 1.2).
%   earth_radius    = Earth radius of curvature (meters, defaults to
%                     6378101.030201019, the nominal value at 45N).
% Outputs:
%   cycle_ranges   = range required to complete one cycle
%                    as function of source angle (meters)
%
function [cycle_ranges,theory_ranges] = pedersen_range_integ_z( source_depth, source_angles, ...
    surface_speed, surface_grad, earth_radius )

% assign default values used in:
% * M. A. Pedersen, D. F. Gordon, "Normal-Mode and Ray Theory Applied
%   to Underwater Acoustic conditions of Extreme Downward Refraction",
%   J. Acoust. Soc. Am. 51 (1B), 323-368 (June 1972).

if ( nargin < 1 ), source_depth=1000.0 ; end ;
if ( nargin < 2 ), source_angles=20:2:50 ; end ;
if ( nargin < 3 ), surface_speed=1550.0 ; end ;
if ( nargin < 4 ), surface_grad=1.2 ; end ;
if ( nargin < 5 ), earth_radius=6378101.030201019 ; end ;

% compute ray parameters

launch_angles = source_angles(:)*pi/180.0 ;
length_angles = length(launch_angles) ;

source_speed = sound_speed(source_depth) ;
ray_param = cos(launch_angles) / source_speed ;

% compute vertex range for each source angle

vertex_ranges = zeros(size(launch_angles)) ;
theory_ranges = zeros(size(launch_angles)) ;
for n = 1:length_angles

    % compute the depth at which ray becomes horizontal

    vertex_speed = 1.0 ./ ray_param(n) ;
    limits = [0 source_depth] ;
    vertex_depth = fzero( @speed_diff, limits ) ;
    
    % compute the range at which vertex occurs

    vertex_ranges(n) = quadgk( @cot_angle, vertex_depth, source_depth ) ;
    
end

% convert to distance units

cycle_ranges = 2 * vertex_ranges ;

% compare to the theory from the paper: R = 2 a^2 / b * tan(A)

a = 1 ./ surface_speed.^2 ;
b = 2.0 * surface_grad ./ surface_speed .^3 ;
theory_ranges = 4/b/(source_speed).^2 ...
    * sin(launch_angles) .* cos(launch_angles) ;

%%
% Compute the Pedersen profile sound speed at a specific depth.
% 
% M. A. Pedersen, D. F. Gordon, "Normal-Mode and Ray Theory Applied
% to Underwater Acoustic conditions of Extreme Downward Refraction",
% J. Acoust. Soc. Am. 51 (1B), 323-368 (June 1972).
%
    function c = sound_speed(z)
        c = surface_speed ...
          ./ sqrt( 1 + 2*surface_grad/surface_speed*z ) ;
%         fprintf( 'd=%.1f c=%.2f\n', (earth_radius-r), c ) ;
    end


%%
% Used to find depth where speed equals that at the vertex
%
    function dc = speed_diff(z)
        c = sound_speed(z) ;
        dc = c - vertex_speed ;
%         fprintf('z=%f c=%f a=%f dc=%f\n',z,c,vertex_speed,dc);
    end

%%
% Snell's law calculation of the cot(angle) at a given depth.
%
    function cot = cot_angle(z)
        cosine = sound_speed(z) / vertex_speed ;
        cot = cosine ./ sqrt( 1.0 - cosine.*cosine ) ;
    end
end