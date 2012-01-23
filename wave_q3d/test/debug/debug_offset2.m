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
% ********* wave_queue::add_eigenray:  time=0.6 de(50)=20 az(3)=0.5
% ( (4947.31,4947.31,5391.25), (9587.87,9587.87,10025.1), (15705.2,15705.2,16135.7) )
% ( (4523.5,4523.5,4974.1), (9253.22,9253.22,9696.83), (15496.4,15496.4,15933) )
% ( (4487.19,4487.19,4944.37), (9297.85,9297.85,9747.8), (15659.4,15659.4,16102.2) )
% ********* wave_queue::add_eigenray:  time=0.61 de(47)=18.5 az(3)=0.5
% ( (519.826,519.826,980.279), (335.047,335.047,796.625), (229.293,229.293,691.95) )
% ( (385.948,385.948,853.448), (199.133,199.133,667.774), (91.9415,91.9415,561.754) )
% ( (648.339,648.339,1122.84), (461.176,461.176,936.879), (354.302,354.302,831.192) )
%
clear all ; close all
disp('*** debug_offset2 ***') ;

dtime = 0.01 * 1000.0 ;
dtheta = 0.5 ;
dphi = 1.0 ;

ray1(1,:,:) = [
    4947.31,4947.31,5391.25; 9587.87,9587.87,10025.1; 15705.2,15705.2,16135.7 ] ;
ray1(2,:,:) = [
    4523.5,4523.5,4974.1; 9253.22,9253.22,9696.83; 15496.4,15496.4,15933 ] ;
ray1(3,:,:) = [
    4487.19,4487.19,4944.37; 9297.85,9297.85,9747.8; 15659.4,15659.4,16102.2 ] ;

ray2(1,:,:) = [
    519.826,519.826,980.279; 335.047,335.047,796.625; 229.293,229.293,691.95 ] ;
ray2(2,:,:) = [
    385.948,385.948,853.448; 199.133,199.133,667.774; 91.9415,91.9415,561.754 ] ;
ray2(3,:,:) = [
    648.339,648.339,1122.84; 461.176,461.176,936.879; 354.302,354.302,831.192 ] ;

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
    
    fprintf( 'grad    = ( %f, %f, %f )\n', grad(1), grad(2), grad(3) ) ;
    fprintf( 'hessian = ( %f, %f, %f )\n', hessian(1,1), hessian(1,2), hessian(1,3) ) ;
    fprintf( '          ( %f, %f, %f )\n', hessian(2,1), hessian(2,2), hessian(2,3) ) ;
    fprintf( '          ( %f, %f, %f )\n', hessian(3,1), hessian(3,2), hessian(3,3) ) ;

    fprintf( 'det     = %f\n', det(hessian) ) ;

    offset = hessian \ grad ;
    fprintf( 'offset  = ( %f, %f, %f )\n', offset(1), offset(2), offset(3) ) ;

    scale = max(grad) ;
    grad = grad / scale ;
    hessian = hessian / scale ;

    fprintf('\n');
    fprintf( 'grad    = ( %f, %f, %f )\n', grad(1), grad(2), grad(3) ) ;
    fprintf( 'hessian = ( %f, %f, %f )\n', hessian(1,1), hessian(1,2), hessian(1,3) ) ;
    fprintf( '          ( %f, %f, %f )\n', hessian(2,1), hessian(2,2), hessian(2,3) ) ;
    fprintf( '          ( %f, %f, %f )\n', hessian(3,1), hessian(3,2), hessian(3,3) ) ;

    fprintf( 'det     = %f\n', det(hessian) ) ;

    offset = hessian \ grad ;
    offset(1) = offset(1) / 1000.0 ;
    fprintf( 'offset  = ( %f, %f, %f )\n', offset(1), offset(2), offset(3) ) ;
end    
disp('*** done ***') ;
