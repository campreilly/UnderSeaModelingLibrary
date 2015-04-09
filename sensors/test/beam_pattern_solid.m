close all ; clear all ;

%% Matlab implementation
[param, desc] = xlsread('solid_pattern_parameters.csv') ;
pitch = param(1) ;
heading = param(2) ;
roll = param(3) ;
dr = pi/180 ;
de = -90:90 ;
az = 0:360 ;
theta = (90-de)*dr ;
phi = az*dr ;
[theta_grid, phi_grid] = meshgrid(theta,phi) ;
pattern = zeros(size(theta_grid)) ;
max_de = 20 ;
min_de = -20 ;
max_phi = 135 ;
min_phi = 45 ;
max_theta = (90 - min_de)*dr ;
min_theta = (90 - max_de)*dr ;
n = find( min_theta <= theta & theta <= max_theta ) ;
m = find( min_phi <= az & az <= max_phi ) ;
pattern(m,n) = 1 ;
db = 20 * log10( abs(pattern) ) ;
db = db + 30 ;
m = find( db < 0 ) ;
db(m) = 0 ;

xx = db .* sin(theta_grid) .* cos(phi_grid) ;
yy = db .* sin(theta_grid) .* sin(phi_grid) ;
zz = db .* cos(theta_grid) ;
r_ = roll * pi/180 ;
p_ = pitch * pi/180 ;
y_ = heading * pi/180 ;
Rx = [ 1 0 0;
       0 cos(p_) sin(p_);
       0 -sin(p_) cos(p_) ] ;
Ry = [ cos(r_) 0 sin(r_);
       0 1 0;
       -sin(r_) 0 cos(r_) ] ;
Rz = [ cos(y_) sin(y_) 0;
       -sin(y_) cos(y_) 0;
       0 0 1 ] ;
R = Rz * Ry * Rx ;
X = xx;
Y = yy;
Z = zz;
[size1_, size2_] = size(xx) ;
for i=1:size1_
    for j=1:size2_
        tmp = [ xx(i,j), yy(i,j), zz(i,j) ] ;
        tmp = R * tmp' ;
        X(i,j) = tmp(1) ;
        Y(i,j) = tmp(2) ;
        Z(i,j) = tmp(3) ;
    end
end
figure ;
surf( X, Y, Z, db, 'EdgeColor','none','FaceColor','interp') ;
colormap(jet) ;
view(-135, 14) ;
set(gca,'Xlim',[-30 30]) ;
set(gca,'Ylim',[-30 30]) ;
set(gca,'Zlim',[-30 30]) ;
xlabel('x') ;
ylabel('y') ;
zlabel('z') ;
title({'Beam Pattern: Solid Angle', ...
       sprintf('Pitch = %0.f Heading = %0.f Roll = %0.f',pitch,heading,roll)}) ;

   
%% Solid angle plot
data1 = xlsread('beam_pattern_solid.csv') ;

db1 = 20.0 * log10( abs(data1) ) ;
db1 = db1 + 30 ;
m = find( db1 < 0 ) ;
db1(m) = 0 ;

de = (0:180)*pi/180 ;    % D/E angles where to evaluate
az = (0:360)*pi/180 ;   % AZ angles where to evaluate
[de_grid,az_grid] = meshgrid(de,az) ;

xx1 = db1 .* cos(az_grid) .* sin(de_grid) ;
yy1 = db1 .* sin(az_grid) .* sin(de_grid) ;
zz1 = db1 .* cos(de_grid) ;
figure ;
surf( xx1, yy1, zz1, db1, 'EdgeColor','none','FaceColor','interp') ;
colormap(jet) ;
view(-135, 14) ;
% view([144 24]) ;
set(gca,'Xlim',[-30 30]) ;
set(gca,'Ylim',[-30 30]) ;
set(gca,'Zlim',[-30 30]) ;
xlabel('x') ;
ylabel('y') ;
zlabel('z') ;
title('Solid Angle Beam Pattern: C++ Implementation') ;