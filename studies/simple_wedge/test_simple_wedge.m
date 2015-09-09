%% test_simple_wedge
% Test eigenray generation capabilities of simple_wedge
%
% This routine compute eigenrays for a receiver co-located with a source in
% a simple wedge.  It writes eigenrays for this scenario to a *.csv file so
% that the geometry of ensonification can be compare to analytic results
% post-test.
%
clear ; close all

%% define scenerio geometry

freq = 110 ;                        % frequency in Hz
sound_speed = 1500 ;                % speed of sound in water (m/s)
wave_number = 2 * pi * freq / sound_speed ;
angle_wedge = 20 * pi / 180.0 ;     % wide angle, 20 degree wedge
range_src = 150.0 ;                 % source slant range from apex
angle_src = angle_wedge / 4.0 ;     % source 1/4 to bottom
range_rcv = range_src ;             % receiver angle matches source
cross_rcv = 1e-6 ;                  % receiver nearly colocated with source
angle_rcv = angle_src ;             % receiver angle matches source

%% compute wedge transmission loss with 20logR removed

[ pressure, eigenrays ] = simple_wedge( wave_number, angle_wedge, ...
    range_rcv, angle_rcv, cross_rcv, range_src, angle_src  ) ;
csvwrite('test_simple_wedge.csv',eigenrays) ;

% test trends for images above surface

for n=1:8
    assert( eigenrays(n+1,1) >= eigenrays(n,1) ) ;  % num bottom bounces
    assert( eigenrays(n+1,2) >= eigenrays(n,2) ) ;  % num surface bounces
    assert( eigenrays(n+1,6) < eigenrays(n,6) ) ;   % range decreasing
    assert( eigenrays(n+1,7) < eigenrays(n,7) ) ;   % D/E decreasing
    assert( eigenrays(n+1,8) > eigenrays(n,8) ) ;   % AZ increasing
    assert( eigenrays(n+1,9) > eigenrays(n,9) ) ;   % TL decreasing
end

% test trends for images below surface

for n=11:17
    assert( eigenrays(n+1,1) >= eigenrays(n,1) ) ;  % num bottom bounces
    assert( eigenrays(n+1,2) >= eigenrays(n,2) ) ;  % num surface bounces
    assert( eigenrays(n+1,6) > eigenrays(n,6) ) ;   % range increasing
    assert( eigenrays(n+1,7) < eigenrays(n,7) ) ;   % D/E decreasing
    assert( eigenrays(n+1,8) < eigenrays(n,8) ) ;   % AZ decreasing
    assert( eigenrays(n+1,9) < eigenrays(n,9) ) ;   % TL increasing
end