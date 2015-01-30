%%
% line_array.m
%     sound_speed   : speed of sound in water at array
%     N             : number of elements
%     spacing       : spacing between elements
%     roll          : amount of roll on the array
%     pitch         : amount of pitch in the theta direction (up positive)
%     yaw           : amount of yaw in the phi direction (clockwise
%                     positive)
%     steering      : steering relative to straight up
%     freq          : signal frequency
% Produces 3-D plot as function of solid angle.
%
function line_pattern( sound_speed, spacing, N, roll, pitch, yaw, steering, freq )

    theta_tilt = pitch ;  % pitch relative to straight up
    phi_tilt = yaw ;
%     theta = (0:180)*pi/180 ;    % D/E angles where to evaluate
%     phi = (-180:180)*pi/180 ;   % AZ angles where to evaluate
%     [theta_grid,phi_grid] = meshgrid(theta,phi) ;
    wavelength = sound_speed/freq ; % signal wavelength

    % compute spherical dot product between 
    % incoming wave directions and the tilted array

    de = (-90:90)*pi/180 ;
    az = (0:360)*pi/180 ;
    [de_grid, az_grid] = meshgrid(de,az) ;

    theta_grid = de_grid + pi/2 ;
    phi_grid = -az_grid ;
    sint = sin( (theta_grid - theta_tilt) / 2 ) ;
    sinp = sin( (phi_grid + phi_tilt) / 2 ) ;
    dotnorm = 1 - 2 * ( sint.*sint ...
        + sin(theta_grid) .* sin(theta_tilt) .* sinp .* sinp ) ;

    % compute beam pattern with steering along array direction

    C = 2 * pi * spacing / wavelength ;
    psi = C * dotnorm ;
    psi_steer = C * sin(steering) ;

    pattern = sin( N*(psi-psi_steer+eps)/2 ) ...
           ./ (N*sin((psi-psi_steer+eps)/2)) ;

    % cpnvert beam pattern into a 3-d surface plot

    dB = 20*log10( abs(pattern) ) ;
    dB = dB+30 ; n = find( dB < 0 ) ; dB(n) = 0 ;
    X = dB .* cos(phi_grid) .* sin(theta_grid) ;
    Y = dB .* sin(phi_grid) .* sin(theta_grid) ;
    Z = dB .* cos(theta_grid) ;
    figure ;
    h = surf(X,Y,Z,dB,'EdgeColor','none','FaceColor','interp') ;
    colormap(jet)
    view([0 0])
    set(gca,'Xlim',[-30 30])
    set(gca,'Ylim',[-30 30])
    set(gca,'Zlim',[-30 30])
    xlabel('x');
    ylabel('y');
    zlabel('z');
    title('Matlab Implementation') ;

end