% florida_bathmetry.m 
%
%
clear all; 
close all;

% % load and plot ETOPO1 data
% 
% figure; 
% etopo1 = load_bathymetry('florida-etop1.nc');
% surf(etopo1.longitude,etopo1.latitude,etopo1.altitude,...
%      'LineStyle','none','FaceColor','interp')
% view([0 90]); colormap(coldhot);
% axis([-80.1 -79.85 26 26.8]);
% set(gca,'TickDir','out')
% set(gca,'Clim',[-350 0])
% xlabel('Latitude (deg)');
% ylabel('Longitude (deg)');
% title('ETOPO1 (60 sec resolution)');
% colorbar;

% load and plot CRM data

figure;
crm = load_arc_ascii('florida-crm.asc')
surf(crm.longitude,crm.latitude,crm.altitude,...
     'LineStyle','none','FaceColor','interp')
view([0 90]); colormap(coldhot);
axis([-80.1 -79.85 26 26.8]);
set(gca,'TickDir','out')
set(gca,'Clim',[-350 0])
xlabel('Latitude (deg)');
ylabel('Longitude (deg)');
title('Coastal Relief Model (3 sec resolution)');
colorbar;

% add ship tracks

hold on;
plot(-79.99054,26.0217,'ko');
x = [ -79.99054 -79.88057 ] ;
y = [ 26.03545 26.73028 ] ;
plot(x,y,'k-');
hold off

% load and plot SVP data

m = csvread('svp_calops_summer_2007.csv');
depth=-400:0;
speed=interp1(-m(:,1),m(:,2),depth,'pchip')
figure; 
plot(m(:,2),-m(:,1),'o',speed,depth,'-');
grid;
axis([1490 1550 -400 0]);
xlabel('Sound Speed (m/s)');
ylabel('Depth (m)');
title('CALOPS RUN 1N SEPT 2007');
