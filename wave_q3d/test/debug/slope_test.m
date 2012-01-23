%
% slope_test - test the accuracy of unit normal calc from ETOPO1 slope
%
% Expected results:
%
%       location: theta=0.942470 phi=0.277340
%       location: lat=36.000447 long=15.890411
%       gradient: theta=0.025020 phi=-0.124374
%       slope: theta=1.433252 phi=-7.089694 deg
%       normal: theta=-0.025012 phi=-0.123113
%
clear all ; close all ;
disp('*****');
disp('slope_test - test the accuracy of unit normal calc from ETOPO1 slope');

earth_radius = 6378101.030201019 ;     % default at lat=45

minlat = 35.9 ; maxlat = 36.1 ;
minlng = 15.75; maxlng = 16.05 ;

mintheta = 0.9405 ; maxtheta = 0.9440 ;
minphi = 0.275; maxphi = 0.280 ;

bathymetry = load_bathymetry('med_ocean.nc') ;
n = find( bathymetry.latitude >= minlat & bathymetry.latitude <= maxlat ) ;
m = find( bathymetry.longitude >= minlng & bathymetry.longitude <= maxlng ) ;
bathymetry.latitude = bathymetry.latitude(n) ;
bathymetry.longitude = bathymetry.longitude(m) ;
bathymetry.altitude = bathymetry.altitude(n,m) ;

figure ;
hb = surf( bathymetry.longitude, bathymetry.latitude, bathymetry.altitude ) ;
set(hb,'EdgeColor','white') ;
xlabel('Longitude (deg)');
ylabel('Latitude (deg)');
zlabel('Depth (m)');
title('Malta Escarpment - Depth');
set(gca,'XLim',[minlng maxlng]);
set(gca,'yLim',[minlat maxlat]);

% convert to spherical polar coordinates

rho = earth_radius + flipud( bathymetry.altitude ) ;
theta = ( 90.0 - flipud(bathymetry.latitude) ) * pi / 180 ;
phi = bathymetry.longitude * pi / 180 ;
h = phi(2) - phi(1) ;

% compute gradients & convert to lengths

[pgrid,tgrid]=meshgrid(phi,theta) ;
[drdp,drdt] = gradient(rho,h) ;
drdt = drdt ./ rho ;
drdp = drdp ./ ( rho .* sin(tgrid) ) ;

figure ;
hr = surf( phi, theta, rho ) ;
set(hr,'EdgeColor','white') ;
xlabel('Phi (rad)');
ylabel('Theta (rad)');
zlabel('Height (m)');
title('Malta Escarpment - Radial Height (m)');
set(gca,'XLim',[minphi maxphi]);
set(gca,'yLim',[mintheta maxtheta]);

figure ;
ht = surf( phi, theta, drdt ) ;
set(ht,'EdgeColor','white') ;
xlabel('Phi (rad)');
ylabel('Theta (rad)');
zlabel('Gradient (m/m)');
title('Malta Escarpment - Gradient in Theta');
set(gca,'XLim',[minphi maxphi]);
set(gca,'yLim',[mintheta maxtheta]);
set(gca,'ZLim',[-0.2 0.2]);

figure ;
hp = surf( phi, theta, drdp ) ;
set(hp,'EdgeColor','white') ;
xlabel('Phi (rad)');
ylabel('Theta (rad)');
zlabel('Depth (m)');
title('Malta Escarpment - Gradient in Phi');
set(gca,'XLim',[minphi maxphi]);
set(gca,'yLim',[mintheta maxtheta]);
set(gca,'ZLim',[-0.2 0.2]);

% interpolate to a sepcific point from malta_rays_test

t = 0.942470 ;
p = 0.27734 ;
fprintf('location: theta=%f phi=%f\n',t,p);
fprintf('location: lat=%f long=%f\n',90-t*180/pi,p*180/pi);
h = interp2( phi, theta, rho, p, t ) - earth_radius ;
dt = interp2( phi, theta, drdt, p, t ) ;
dp = interp2( phi, theta, drdp, p, t ) ;
fprintf('gradient: theta=%f phi=%f\n',dt,dp);
slope_t = atan(dt) ;
slope_p = atan(dp) ;
normal_t = -sin(slope_t) ;
normal_p = atan(slope_p) ;
fprintf('depth: h=%f\n',h);
fprintf('slope: theta=%f phi=%f deg\n',slope_t*180/pi,slope_p*180/pi);
fprintf('normal: theta=%f phi=%f\n',normal_t,normal_p);

