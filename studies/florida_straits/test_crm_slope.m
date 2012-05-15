%%
% Compares slopes computed using Matlab's gradient function
% to those produced by USML.  Used to search for flaws in the
% calculation of slope and surface normal.
%
% clear all; 
close all
earth_radius = 6378101.030201019 ;
scale = [-80.2 -79.7 26 27 ] ;
viewang = [0 90] ;
earth_radius = 6378101.030201019 ;

% load bathymetry from disk

% bathymetry = load_arc_ascii('flstrts_bathymetry.asc');
figure; 
surf(bathymetry.longitude,bathymetry.latitude,bathymetry.altitude,'FaceColor','interp','LineStyle','none')
colormap(coldhot);
axis(scale); view(viewang); colorbar;
% zlim = [ 0 8 ] ;
% set(gca,'ZLim',zlim,'CLim',zlim);
xlabel('Longitude (deg)') ;
ylabel('Latitude (deg)') ;
title('Coastal Relief Model (m)');

% compute latitude and longitude components of gradient
% using the adacent differences between depths

alt = earth_radius + bathymetry.altitude ;
lat = (90-bathymetry.latitude)*pi/180;
lng = bathymetry.longitude*pi/180;

num_lng = length( lng ) ;
grad_lng = diff( alt' )' / ( lng(2)-lng(1) ) ;
grad_lng = [ grad_lng grad_lng(:,num_lng-1) ] ;

num_lat = length( lat ) ;
grad_lat = diff( alt ) / ( lat(2)-lat(1) ) ;
grad_lat = [ grad_lat ; grad_lng(num_lat-1,:) ] ;

% convert gradients to slopes, and slopes to slope angles

[x,y] = meshgrid(lng,lat);
slope_lng = grad_lng ./ ( alt .* sin(y) ) ;
slope_lat = grad_lat ./ alt ;

angle_lng = atan( slope_lng ) * 180 / pi ;
angle_lat = atan( slope_lat ) * 180 / pi ;
angle_abs = sqrt( angle_lng .* angle_lng + angle_lat .* angle_lat ) ;

figure; 
surf(bathymetry.longitude,bathymetry.latitude,angle_abs,'FaceColor','interp','LineStyle','none')
colormap(flipud(coldhot));
axis(scale); view(viewang); colorbar;
zlim = [ 0 8 ] ;
set(gca,'ZLim',zlim,'CLim',zlim);
xlabel('Longitude (deg)') ;
ylabel('Latitude (deg)') ;
title('Slope Angle (deg)');

% convert slopes in unit normals

norm_lng = - slope_lng ./ sqrt( 1 + slope_lng.*slope_lng ) ;
norm_lat = - slope_lat ./ sqrt( 1 + slope_lat.*slope_lat ) ;
norm_abs = sqrt( norm_lng .* norm_lng + norm_lat .* norm_lat ) ;

x = 1:10:length(lng) ;
y = 1:20:length(lat) ;
u = norm_lng(y,x) ; % ./ norm_abs(y,x) ;
v = norm_lat(y,x) ; % ./ norm_abs(y,x) ; ;

figure; 
h = quiver( bathymetry.longitude(x), bathymetry.latitude(y), u, v, ...
    0.9,'MaxHeadSize',0.01 ) ;
axis(scale); 
xlabel('Longitude (deg)') ;
ylabel('Latitude (deg)') ;
title('Surface Normal');
