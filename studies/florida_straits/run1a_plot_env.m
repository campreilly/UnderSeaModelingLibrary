%%
% run1a_plot_env.m - Plot environment for CALOPS-S Run 1A
%
% In the first plot, this routine overlays the area bathymetry with
% receiver location, source track, and sediment boundary.
% The second plot shows the sound velocity profile.
%
clear all; 
close all;

% load and plot batyhymetry data

bathymetry = load_arc_ascii('flstrts_bathymetry.asc');

figure;
run1a_plot_bathy( bathymetry ) ;
axis([-80.1 -79.85 26 26.8]);
title('CALOPS RUN 1N SEPT 2007');

hold on ;
[c,h] = contour(bathymetry.longitude,bathymetry.latitude,bathymetry.altitude,[0 -236.0]);
set(h,'LineColor','black');
hold off ;

print -dpng run1a_bathymetry

% load and plot SVP data

m = csvread('flstrts_profile_sept2007.csv');
depth=-400:0;
speed=interp1(-m(:,1),m(:,2),depth,'pchip');
figure; 
plot(m(:,2),-m(:,1),'o',speed,depth,'-','LineWidth',1.5);
grid;
axis([1490 1550 -400 0]);
xlabel('Sound Speed (m/s)');
ylabel('Depth (m)');
title('CALOPS RUN 1N SEPT 2007');

print -dpng run1a_profile
