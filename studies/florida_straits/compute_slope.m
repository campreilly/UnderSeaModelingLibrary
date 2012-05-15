%%
% Compares slopes computed using Matlab's gradient function
% to those produced by USML.  Used to search for flaws in the
% calculation of slope and surface normal.
%
clear all; close all
scale = [-80.2 -79.7 26 27 ] ;
viewang = [90 0] ;
zlim = [-0.15 0.15] ;

% compute latitude and longitude components of slope
% using Matlab's gradient function

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
surf(bathymetry.longitude,bathymetry.latitude,slat,'FaceColor','interp','LineStyle','none')
colormap(coldhot);
axis(scale); view(viewang); colorbar;
set(gca,'ZLim',zlim,'CLim',zlim);
xlabel('Longitude (deg)') ;
ylabel('Latitude (deg)') ;
title('Latitude component of slope (Matlab)');

figure; 
surf(bathymetry.longitude,bathymetry.latitude,slng,'FaceColor','interp','LineStyle','none')
colormap(coldhot);
axis(scale); view(viewang); colorbar;
set(gca,'ZLim',zlim,'CLim',zlim);
xlabel('Longitude (deg)') ;
ylabel('Latitude (deg)') ;
title('Longitude component of slope (Matlab)');

% compute the depth and slope at a specific point

[p,m] = min(abs(bathymetry.longitude+79.99054)) ;
% [p,m] = min(abs(bathymetry.longitude+80.068)) ;
[p,n] = min(abs(bathymetry.latitude-26.0217)) ;
fprintf( 'n,m=%d,%ddepth=%f glat=%f glng=%f slat=%f slng=%f nlat=%f nlng=%f\n', ...
    n, m, bathymetry.altitude(n,m), glat(n,m), glng(n,m), ...
    slat(n,m), slng(n,m), nlat(n,m), nlng(n,m) ) ;

% compare latitude component of surface normal
% from Matlab to that produced by USML

% figure; 
% surf(bathymetry.longitude,bathymetry.latitude,nlat,'FaceColor','interp','LineStyle','none')
% colormap(coldhot);
% axis(scale); view(viewang); colorbar;
% set(gca,'ZLim',zlim,'CLim',zlim);
% xlabel('Longitude (deg)') ;
% ylabel('Latitude (deg)') ;
% title('Latitude component of surface normal (Matlab)');
% 
nlat2=csvread('flstrts_slope_normlat.csv');
nlat2=nlat2(:,1:length(bathymetry.longitude));

% figure; 
% surf(bathymetry.longitude,bathymetry.latitude,nlat2,'FaceColor','interp','LineStyle','none')
% colormap(coldhot);
% axis(scale); view(viewang); colorbar;
% set(gca,'ZLim',zlim,'CLim',zlim);
% xlabel('Longitude (deg)') ;
% ylabel('Latitude (deg)') ;
% title('Latitude component of surface normal (USML)');

figure; 
surf(bathymetry.longitude,bathymetry.latitude,nlat2-nlat,'FaceColor','interp','LineStyle','none')
colormap(coldhot);
axis(scale); view(viewang); colorbar;
% set(gca,'ZLim',zlim,'CLim',zlim);
xlabel('Longitude (deg)') ;
ylabel('Latitude (deg)') ;
title('Error in latitude component of surface normal (USML-Matlab)');

% compare longtidue component of surface normal
% from Matlab to that produced by USML

% figure; 
% surf(bathymetry.longitude,bathymetry.latitude,nlng,'FaceColor','interp','LineStyle','none')
% colormap(coldhot);
% axis(scale); view(viewang); colorbar;
% xlabel('Longitude (deg)') ;
% ylabel('Latitude (deg)') ;
% set(gca,'ZLim',zlim,'CLim',zlim);
% title('Longitude component of surface normal (Matlab)');

nlng2=csvread('flstrts_slope_normlng.csv');
nlng2=nlng2(:,1:length(bathymetry.longitude));

% figure; 
% surf(bathymetry.longitude,bathymetry.latitude,nlng2,'FaceColor','interp','LineStyle','none')
% colormap(coldhot);
% axis(scale); view(viewang); colorbar;
% xlabel('Longitude (deg)') ;
% ylabel('Latitude (deg)') ;
% set(gca,'ZLim',zlim,'CLim',zlim);
% title('Longitude component of surface normal (USML)');

figure; 
surf(bathymetry.longitude,bathymetry.latitude,nlng2-nlng,'FaceColor','interp','LineStyle','none')
colormap(coldhot);
axis(scale); view(viewang); colorbar;
xlabel('Longitude (deg)') ;
ylabel('Latitude (deg)') ;
% set(gca,'ZLim',zlim,'CLim',zlim);
title('Error in longitude component of surface normal (USML-Matlab)');

figure; 
plot(bathymetry.latitude,nlat(:,m),'o',bathymetry.latitude,nlat2(:,m),'o',...
     bathymetry.latitude,nlng(:,m),'o',bathymetry.latitude,nlng2(:,m),'o' ) ;
grid ; set(gca,'YLim',zlim) ;
xlabel('Latitude (deg)') ;
ylabel('Surface Normal') ;
legend('lat matlab','lat usml','long matlab','long usml','Location','Best');

figure; 
plot(bathymetry.longitude,nlat(n,:),'o',bathymetry.longitude,nlat2(n,:),'o',...
     bathymetry.longitude,nlng(n,:),'o',bathymetry.longitude,nlng2(n,:),'o' ) ;
grid ; set(gca,'YLim',zlim) ;
xlabel('Longitude (deg)') ;
ylabel('Surface Normal') ;
legend('lat matlab','lat usml','long matlab','long usml','Location','Best');
