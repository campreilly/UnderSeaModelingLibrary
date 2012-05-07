clear all; close all
scale = [-80.25 -79.75 26 27 ] ;

bathymetry = load_arc_ascii('flstrts_bathymetry.asc');
alt = 6378101.030201019 + bathymetry.altitude ;
lat=(90-bathymetry.latitude)*pi/180;
lng=bathymetry.longitude*pi/180;
[glng,glat]=gradient(alt,lng,lat);
[x,y] = meshgrid(lng,lat);
slng = glng ./ ( alt .* sin(y) ) ;
slat = glat ./ alt ;
nlat = -slat ./ sqrt( 1+slat.^2) ;
nlng = -slng ./ sqrt( 1+slng.^2) ;

figure; 
surf(bathymetry.longitude,bathymetry.latitude,slat,'LineStyle','none')
colormap(coldhot);
view([0 90]); colorbar;
axis(scale);
% set(gca,'CLim',[-0.04 0.06]);
xlabel('Longitude (deg)') ;
ylabel('Latitude (deg)') ;
title('Latitude component of slope');

figure; 
surf(bathymetry.longitude,bathymetry.latitude,slng,'LineStyle','none')
colormap(coldhot);
view([0 90]); colorbar;
axis(scale);
% set(gca,'CLim',[-0.04 0.06]);
xlabel('Longitude (deg)') ;
ylabel('Latitude (deg)') ;
title('Longitude component of slope');

% 
% [p,m] = min(abs(bathymetry.longitude+79.99054)) ;
% [p,n] = min(abs(bathymetry.latitude-26.0217)) ;
% fprintf( 'n,m=%d,%ddepth=%f glat=%f glng=%f slat=%f slng=%f nlat=%f nlng=%f\n', ...
%     n, m, bathymetry.altitude(n,m), glat(n,m), glng(n,m), ...
%     slat(n,m), slng(n,m), nlat(n,m), nlng(n,m) ) ;
% 
% figure; 
% surf(bathymetry.longitude,bathymetry.latitude,nlat,'LineStyle','none')
% colormap(coldhot);
% view([0 90]); colorbar;
% axis(scale);
% set(gca,'CLim',[-0.04 0.06]);
% xlabel('Longitude (deg)') ;
% ylabel('Latitude (deg)') ;
% title('Latitude component of surface normal');
% 
% nlat2=csvread('flstrts_slope_normlat.csv');
% nlat2=nlat2(:,1:length(bathymetry.longitude));
% 
% figure; 
% surf(bathymetry.longitude,bathymetry.latitude,nlat2,'LineStyle','none')
% colormap(coldhot);
% view([0 90]); colorbar;
% axis(scale);
% set(gca,'CLim',[-0.04 0.06]);
% xlabel('Longitude (deg)') ;
% ylabel('Latitude (deg)') ;
% title('Latitude component of surface normal');
% 
% figure; 
% surf(bathymetry.longitude,bathymetry.latitude,nlng,'LineStyle','none')
% colormap(coldhot);
% view([0 90]); colorbar;
% axis(scale);
% xlabel('Longitude (deg)') ;
% ylabel('Latitude (deg)') ;
% set(gca,'CLim',[-0.15 0.25]);
% title('Longitude component of surface normal');
% 
% nlng2=csvread('flstrts_slope_normlng.csv');
% nlng2=nlng2(:,1:length(bathymetry.longitude));
% figure; 
% surf(bathymetry.longitude,bathymetry.latitude,nlng2,'LineStyle','none')
% colormap(coldhot);
% view([0 90]); colorbar;
% axis(scale);
% xlabel('Longitude (deg)') ;
% ylabel('Latitude (deg)') ;
% set(gca,'CLim',[-0.15 0.25]);
% title('Longitude component of surface normal');
% 
% 
