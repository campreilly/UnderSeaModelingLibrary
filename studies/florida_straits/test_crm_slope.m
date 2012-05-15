%%
% Estimates bottom slope of a Coastal Relief Model bathymetry
% dataset. Used to search for flaws in the calculation of slope 
% and surface normal.
%
clear all; 
close all
disp('=== testcrm_slope ===');
earth_radius = 6378101.030201019 ;
scale = [-80.2 -79.7 26 27 ] ;
viewang = [0 90] ;
earth_radius = 6378101.030201019 ;
% test_lat = 26.35 ;
% test_lng = -79.91 ;
test_lat = 26.0667 ;
test_lng = -80.075 ;

% load bathymetry from disk

bathymetry = load_arc_ascii('flstrts_bathymetry.asc');
figure; 
surf(bathymetry.longitude,bathymetry.latitude,bathymetry.altitude,'FaceColor','interp','LineStyle','none')
colormap(coldhot);
axis(scale); view(viewang); colorbar;
xlabel('Longitude (deg)') ;
ylabel('Latitude (deg)') ;
title('Coastal Relief Model (m)');
[p,index_lng] = min( abs(bathymetry.longitude-test_lng) ) ;
[p,index_lat] = min( abs(bathymetry.latitude-test_lat) ) ;
fprintf('latitude(%d)=%f longitude(%d)=%f\n', ...
    index_lat, bathymetry.latitude(index_lat), ...
    index_lng, bathymetry.longitude(index_lng) ) ;
    
% compute latitude and longitude components of gradient
% using the adacent differences between depths

alt = earth_radius + bathymetry.altitude ;
lat = (90-bathymetry.latitude)*pi/180;
lng = bathymetry.longitude*pi/180;

num_lng = length( lng ) ;
diff_lng = diff( alt' )' ;
grad_lng = diff_lng / ( lng(2)-lng(1) ) ;
grad_lng = [ grad_lng grad_lng(:,num_lng-1) ] ;

num_lat = length( lat ) ;
diff_lat = diff( alt ) ;
grad_lat = diff_lat / ( lat(2)-lat(1) ) ;
grad_lat = [ grad_lat ; grad_lng(num_lat-1,:) ] ;

fprintf('diff: lat=%f lng=%f\n', ...
    diff_lat(index_lat,index_lng), diff_lng(index_lat,index_lng) ) ;

fprintf('gradient: lat=%f lng=%f\n', ...
    grad_lat(index_lat,index_lng), grad_lng(index_lat,index_lng) ) ;

% convert gradients to slopes, and slopes to slope angles

[x,y] = meshgrid(lng,lat);
slope_lng = grad_lng ./ ( alt .* sin(y) ) ;
slope_lat = grad_lat ./ alt ;
fprintf('slope: lat=%f lng=%f\n', ...
    slope_lat(index_lat,index_lng), slope_lng(index_lat,index_lng) ) ;

angle_lng = atan( slope_lng ) * 180 / pi ;
angle_lat = atan( slope_lat ) * 180 / pi ;
angle_abs = sqrt( angle_lng .* angle_lng + angle_lat .* angle_lat ) ;
fprintf('angle: lat=%f lng=%f abs=%f\n', ...
    angle_lat(index_lat,index_lng), angle_lng(index_lat,index_lng), ...
    angle_abs(index_lat,index_lng) ) ;

figure; 
surf(bathymetry.longitude,bathymetry.latitude,angle_abs,'FaceColor','interp','LineStyle','none')
colormap(flipud(coldhot));
axis(scale); view(viewang); colorbar;
zlim = [ 0 8 ] ;
set(gca,'ZLim',zlim,'CLim',zlim);
xlabel('Longitude (deg)') ;
ylabel('Latitude (deg)') ;
title('Slope Angle (deg)');
print -dpng test_crm_slope_angle

% convert slopes in unit normals

norm_lng = - slope_lng ./ sqrt( 1 + slope_lng.*slope_lng ) ;
norm_lat = - slope_lat ./ sqrt( 1 + slope_lat.*slope_lat ) ;
norm_abs = sqrt( norm_lng .* norm_lng + norm_lat .* norm_lat ) ;
fprintf('normal: lat=%f lng=%f max(abs())=%f\n', ...
    norm_lat(index_lat,index_lng), norm_lng(index_lat,index_lng), ...
    max(abs(norm_abs(:))) ) ;

x = 1:15:length(lng) ;
y = 1:30:length(lat) ;
u = norm_lng(y,x) ./ norm_abs(y,x) ;
v = -norm_lat(y,x) ./ norm_abs(y,x) ;

figure; 
quiver( bathymetry.longitude(x), bathymetry.latitude(y), u, v, 0.35 ) ;
axis(scale); 
xlabel('Longitude (deg)') ;
ylabel('Latitude (deg)') ;
title('Surface Normal Direction');
print -dpng test_crm_slope_norm
