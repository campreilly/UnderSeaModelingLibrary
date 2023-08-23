%%
% Test Matlab versions of spherical operations adapted from 
% USML's usml::types::wvector.  Reproduces the tests used in C++
%
function test_spherical

dot_test() ;
distance_test() ;

end

%%
% Compute the dot product between set of vectors at different latitudes
% and a vector pointing due north.  If correct, the dots products
% should correspond to the angles defined by the latitudes of
% the original points.
%
function dot_test
    fprintf('*** dot_test ***\n');

    % conversion constants

    d2r = pi/180 ;
    earth_radius = 6371000.0 ;

    % define a series of lat/long points on earth's surface

    n=0:9 ;
    points.latitude = 10.0 * n ;
    points.longitude = 90.0 + 10.0 * n ;
    points.rho = earth_radius ;
    points.theta = (90-points.latitude)*d2r ;
    points.phi = points.longitude*d2r ;

    % compute the distance to some point on earth's surface

    north.latitude = 90.0 ;
    north.longitude = 0.0 ;
    north.rho = earth_radius  ;
    north.theta = (90-north.latitude)*d2r ;
    north.phi = north.longitude*d2r ;
    
    dotnorm = spherical_dotnorm( points, north ) ;
    angle = acos( dotnorm ) / d2r ;

    % check the answer

    for n=1:10
        analytic = 90 - points.latitude(n) ;
        fprintf('lat=%.f\tanalytic=%f\tcomputed=%f\n', ...
            points.latitude(n), analytic, angle(n) ) ;
        if ( abs(angle(n)-analytic) > 1e-6 )
            error('test failed');
        end
    end
end


%%
% Compute the straight line distance between a wposition vector
% at different latitudes and the point 40N 45E. The results are
% compared to the analytic solution
% 
%          distance^2 = 2 R^2 ( 1-cos(40-latitude) )
% 
% Generate errors if values differ by more that 1E-6 meters.  The one
% expection to this accuracy is the case where distance is almost zero.
% In that case, the calculation accuracy is limited to 0.02 meters.
%
function distance_test
    fprintf('*** distance_test ***\n');
    
    % conversion constants

    earth_radius = 6371000.0 ;
    d2r = pi/180 ;

    % define a series of lat/long points on earth

    n=0:9 ;
    points.latitude = 10.0 * n ;
    points.longitude = 45 * ones(size(n)) ;
    points.rho = earth_radius ;
    points.theta = (90-points.latitude)*d2r ;
    points.phi = points.longitude*d2r ;

    % compute the distance to some point on earth

    origin.latitude = 40.0 ;
    origin.longitude = 45.0 ;
    origin.rho = earth_radius ;
    origin.theta = (90-origin.latitude)*d2r ;
    origin.phi = origin.longitude*d2r ;
    
    dist2 = spherical_dist2( points, origin ) ;

    % check the answer
    % distance^2 = 2 R^2 ( 1-cos(angle) )

    for n=0:9
        analytic = 2 * earth_radius^2 ...
            * ( 1 - cos( (40-10*n)*d2r ) ) ;
        fprintf('lat=%.f\tanalytic=%f\tcomputed=%f\n', ...
            points.latitude(n+1), analytic, dist2(n+1) ) ;
        if ( abs(dist2(n+1)-analytic) > 0.1 )
            error('test failed');
        end
    end
end
