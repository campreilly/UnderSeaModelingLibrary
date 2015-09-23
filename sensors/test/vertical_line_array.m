%% Vertical line array's beam pattern as via C++ implementation

close all ; %clear all ;

sheet=importdata('vertical_array_parameters.csv',',');
param = sheet.data ;
sound_speed = param(1) ;
spacing = param(2) ;
N = param(3) ;
pitch = param(4) ;
heading = param(5) ;
roll = param(6) ;
steering = param(7) ;
freq = param(8:end) ;
ax = [0,0,1] ;
line_pattern( sound_speed, spacing, N, pitch, heading, roll, steering, freq, ax ) ;

data=importdata('vertical_array_beam_pattern.csv',',');

db = 10.0 * log10( abs(data) ) ;
% db = flip(db,2) ;
db = db + 30 ;
m = find( db < 0 | isnan(db) ) ;
db(m) = 0 ;

theta = (0:1:180)*pi/180 ;    % D/E angles where to evaluate
phi = (0:360)*pi/180 ;   % AZ angles where to evaluate
[theta_grid,phi_grid] = meshgrid(theta,phi) ;

xx = db .* cos(phi_grid) .* sin(theta_grid) ;
yy = db .* sin(phi_grid) .* sin(theta_grid) ;
zz = db .* cos(theta_grid) ;
figure ;
surf( xx, yy, zz, db, 'EdgeColor','none','FaceColor','interp') ;
colormap(jet) ;
view([0 90]) ;
set(gca,'Xlim',[-30 30]) ;
set(gca,'Ylim',[-30 30]) ;
set(gca,'Zlim',[-30 30]) ;
xlabel('East(+)/West(-)') ;
ylabel('North(+)/South(-)') ;
zlabel('Up') ;
title('C++ Implementation') ;
