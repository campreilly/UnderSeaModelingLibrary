%
% eigenray_concave_rays.m - plot ray paths on a spherical earth
%
clear all ; close all;
incangle = 1 ;              % plot every ray
az = 0 ;                    % search for rays pointing north
R = 6378101.030201019 ;     % earth radius at 45 deg north

% load ray paths and search for paths to plot

wavefront = load_wavefront('eigenray_concave_wave.nc') ;
[p,m] = min( abs( wavefront.source_az - az ) ) ;
n = 1:incangle:length(wavefront.source_de) ;

% convert rays to cartesian coordinates

theta = (squeeze(wavefront.latitude(:,n,m))-45.0) * pi / 180 ;
rho = R + squeeze(wavefront.altitude(:,n,m)) ;
ray_x = rho .* sin( theta ) / 1000.0 ;
ray_y = rho .* cos( theta ) - R ;

% display ray paths on a flat earth

figure ;
plot( theta*180/pi, rho-R, 'k-' )
grid ;
set(gca,'XLim',[0 1.6])
xlabel('Latitude Change (deg)');
ylabel('Depth (m)');
print -deps eigenray_concave_rays1.eps

% compute earth's surface in cartesian coordinates

surf_t = (0:0.01:2)*pi/180 ;
surf_x = R * sin( surf_t ) / 1000.0 ;
surf_y = R * cos( surf_t ) - R ;

% display ray paths on a curved earth

figure ;
plot( surf_x, surf_y, 'k-', ray_x, ray_y, 'k-' ) ;
grid ;
set(gca,'Ylim',[-2000 0]);
set(gca,'XLim',[0 160.0])
xlabel('Range (km)');
ylabel('Depth (m)');
print -deps eigenray_concave_rays2.eps
