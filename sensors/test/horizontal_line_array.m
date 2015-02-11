%% Horizontal line array's beam pattern as via C++ implementation

close all ; clear all ;

[param, desc] = xlsread('horizontal_array_parameters.csv') ;
c0 = param(1) ;
d = param(2) ;
N = param(3) ;
pitch = param(4) + pi/2 ;
yaw = param(5) ;
steering = param(6) ;
freq = param(7:end) ;
line_pattern( c0, d, N, pitch, yaw, steering, freq ) ;

data = xlsread('horizontal_array_beam_pattern.csv') ;

db = 10.0 * log10( abs(data) ) ;
db = db + 30 ;
m = find( db < 0 ) ;
db(m) = 0 ;

theta = (0:180)*pi/180 ;    % D/E angles where to evaluate
phi = (-180:180)*pi/180 ;   % AZ angles where to evaluate
[theta_grid,phi_grid] = meshgrid(theta,phi) ;

xx = db .* cos(phi_grid) .* sin(theta_grid) ;
yy = db .* sin(phi_grid) .* sin(theta_grid) ;
zz = db .* cos(theta_grid) ;
figure ;
surf( xx, yy, zz, db, 'EdgeColor','none','FaceColor','interp') ;
colormap(jet) ;
view([0 0]) ;
set(gca,'Xlim',[-30 30]) ;
set(gca,'Ylim',[-30 30]) ;
set(gca,'Zlim',[-30 30]) ;
xlabel('x') ;
ylabel('y') ;
zlabel('z') ;
title('C++ Implementation') ;
