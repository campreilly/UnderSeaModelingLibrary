% plot results of all proploss_test sub-tests
%
clear all ; close all

% Compares modeled propagation loss as a function of range to the Lloyd's 
% mirror analytic expression for surface reflection in an isovelocity ocean.
 
[data,desc] = xlsread('proploss_lloyds_range.csv');
n = 2:3 ;

figure ;
h = plot( data(:,1)/1e3, data(:,2), 'ko', data(:,1)/1e3, data(:,3), 'k-' ) ;
set(h(1),'MarkerSize',3,'MarkerFaceColor','black')
set(h(2),'LineWidth', 1.25)
grid ;
set(gca,'Ylim',[-110 -30]);
ylabel('Propagation Loss (dB)')
xlabel('Range (km)')
title('Source Depth=25 m Target Depth=200 m Freq=2000 Hz');
legend('WaveQ3D','theory');
print -deps proploss_lloyds_range.eps

figure ;
h = plot( data(:,1)/1e3, data(:,2), 'ko', data(:,1)/1e3, data(:,3), 'k-' ) ;
set(h(1),'MarkerSize',3,'MarkerFaceColor','black')
set(h(2),'LineWidth', 1.25)
grid ;
set(gca,'Xlim',[0 2]);
set(gca,'Ylim',[-110 -30]);
ylabel('Propagation Loss (dB)')
xlabel('Range (km)')
title('Source Depth=25 m Target Depth=200 m Freq=2000 Hz');
legend('WaveQ3D','theory');
print -deps proploss_lloyds_range_zoom.eps

% Compares modeled propagation loss as a function of depth to the Lloyd's 
% mirror analytic expression for surface reflection in an isovelocity ocean.
% This forces the model to deal with target points near the surface where the
% up-going and down-going wavefronts must be extrapolated from two ray 
% families that have different numbers of surface bounces.

[data,desc] = xlsread('proploss_lloyds_depth.csv');
n = 2:3 ;

figure ;
h = plot( data(:,2), data(:,1), 'ko', data(:,3), data(:,1), 'k-' ) ;
set(h(1),'MarkerSize',3,'MarkerFaceColor','black')
set(h(2),'LineWidth', 1.25)
grid ;
set(gca,'Xlim',[-110 -75]);
set(gca,'Ylim',[-40 0]);
xlabel('Propagation Loss (dB)')
ylabel('Depth (m)')
title('Source Depth=25 m Target Range=10 km Freq=2000 Hz');
legend('WaveQ3D','theory');
% print -deps proploss_lloyds_depth.eps
