% debug_offset2.m
%
% Caustic test not rolling off into shadow zone in the 
% same way as the analytic solution.
%  -- Eigenray solution seems to be unstable when point is 
%     outside the bopunding box of 27 points.
%  -- Is it that the Hessian and gradient are not calculated correctly?
%  -- Or, is that the matrix back substitution is unstable?
%
%  Focus proploss test cases to find/fix this problem.
%
% ********* wave_queue::add_eigenray:  time=0.604 de(50)=0.5 az(5)=0.5
% ( (4264.57,4207.96,4264.57), (9466.28,9410.62,9466.28), (14644.5,14589.5,14644.5) )
% ( (4254.51,4197.8,4254.51), (9465.95,9410.2,9465.95), (14652.6,14597.6,14652.6) )
% ( (4248.31,4191.53,4248.31), (9469.39,9413.57,9469.39), (14664.4,14609.3,14664.4) )
% gradient(0) = ( 9413.57 - 9410.62 ) / 0.002
% gradient(1) = ( 14597.6 - 4197.8 ) / 1
% gradient(2) = ( 9465.95 - 9465.95 ) / 1
% hessian:  [3,3]((3.80206e+06,18110.8,-3.24185e-14),(18110.8,-99.9788,0),(-3.24185e-14,0,446))
% gradient: [3](1477.11,10399.8,0)
% offset:   [3](-0.26619,55.8006,-1.93486e-17)
%
clear all ; close all
disp('*** debug_offset2 ***') ;

dtime = 0.001 ;
dtheta = 0.5 ;
dphi = 0.5 ;

ray1(1,:,:) = [
    4264.57,4207.96,4264.57; 9466.28,9410.62,9466.28; 14644.5,14589.5,14644.5 ] ;
ray1(2,:,:) = [
    4254.51,4197.8,4254.51; 9465.95,9410.2,9465.95; 14652.6,14597.6,14652.6 ] ;
ray1(3,:,:) = [
    4248.31,4191.53,4248.31; 9469.39,9413.57,9469.39; 14664.4,14609.3,14664.4 ] ;

ray2(1,:,:) = [
    384.213,326.041,384.213; 197.868,139.556,197.868; 90.9763,32.5076,90.9763 ] ;
ray2(2,:,:) = [
    380.795,322.53,380.795; 194.281,135.875,194.281; 87.2897,28.7428,87.2897 ] ;
ray2(3,:,:) = [
    381.347,322.988,381.347; 194.695,136.18,194.695; 87.6075,28.9669,87.6075 ] ;

for loop=1:2
    if ( loop == 1 )
        distance2 = ray1 ;
    else    
        distance2 = ray2 ;
    end
    % distance2 = sqrt( distance2 ) ;
    fprintf('\n');

    % compute gradients in all directions

    grad(1,1) = ( distance2(3,2,2) - distance2(1,2,2) ) / ( 2 * dtime ) ;
    grad(2,1) = ( distance2(2,3,2) - distance2(2,1,2) ) / ( 2 * dtheta ) ;
    grad(3,1) = ( distance2(2,2,3) - distance2(2,2,1) ) / ( 2 * dphi ) ;
    fprintf( 'grad    = ( %f, %f, %f )\n', grad(1), grad(2), grad(3) ) ;

    % compute second derivatives

    hessian(1,1) = ( distance2(3,2,2) + distance2(1,2,2) - 2*distance2(2,2,2) ) / ( dtime*dtime ) ;
    hessian(2,2) = ( distance2(2,3,2) + distance2(2,1,2) - 2*distance2(2,2,2) ) / ( dtheta*dtheta ) ;
    hessian(3,3) = ( distance2(2,2,3) + distance2(2,2,1) - 2*distance2(2,2,2) ) / ( dphi*dphi ) ;

    hessian(1,2) = ( ... 
        ( distance2(3,3,2) - distance2(1,3,2) ) / (2*dtime) - ...
        ( distance2(3,1,2) - distance2(1,1,2) ) / (2*dtime) ) / 2*dtheta ;
    hessian(2,1) = hessian(1,2) ;

    hessian(1,3) = ( ...
        ( distance2(3,2,3) - distance2(1,2,3) ) / (2*dtime) - ...
        ( distance2(3,2,1) - distance2(1,2,1) ) / (2*dtime) ) / 2*dphi ;
    hessian(3,1) = hessian(1,3) ;

    hessian(2,3) = ( ...
        ( distance2(2,3,3) - distance2(2,1,3) ) / (2*dtime) - ...
        ( distance2(2,3,1) - distance2(2,1,1) ) / (2*dtime) ) / 2*dtheta ;
    hessian(3,2) = hessian(2,3) ;

    fprintf( 'hessian = ( %f, %f, %f )\n', hessian(1,1), hessian(1,2), hessian(1,3) ) ;
    fprintf( '          ( %f, %f, %f )\n', hessian(2,1), hessian(2,2), hessian(2,3) ) ;
    fprintf( '          ( %f, %f, %f )\n', hessian(3,1), hessian(3,2), hessian(3,3) ) ;

    fprintf( 'det     = %f\n', det(hessian) ) ;

    offset = hessian \ grad ;
    fprintf( 'offset  = ( %f, %f, %f )\n', offset(1), offset(2), offset(3) ) ;

end    
disp('*** done ***') ;
