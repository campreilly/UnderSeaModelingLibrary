% plot results of all proploss_test sub-tests
%
clear all ; close all

% Compares modeled propagation loss as a function of range to the Lloyd's 
% mirror analytic expression for surface reflection in an isovelocity ocean.
 
sheet=importdata('proploss_lloyds_range.csv',',');
data = sheet.data ;
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

sheet=importdata('proploss_lloyds_depth.csv',',');
data = sheet.data ;
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
print -deps proploss_lloyds_depth.eps

% Compares modeled propagation loss as a function of range to the Lloyd's 
% mirror analytic expression at multiple frequencies

[p, e] = load_netcdf_proploss('proploss_lloyds_range_freq.nc') ;
size = length(p.latitude) ;     % Number of targets
sheet=importdata('proploss_lloyds_range_freq.csv',',');
data = sheet.data ;
desc = sheet.colheaders ;
range = data(1:981,2) / 1e3 ;
% indices for the various frequencies
freq = [ 1:size ;
         (size+1):(2*size) ;
         (2*size+1):(3*size) ;
         (3*size+1):(4*size) ] ;
% extract the model and theory data for each freq
theory_10hz = data(freq(1,:),4) ;
model_10hz = data(freq(1,:),3) ;
theory_100hz = data(freq(2,:),4) ;
model_100hz = data(freq(2,:),3) ;
theory_1khz = data(freq(3,:),4) ;
model_1khz = data(freq(3,:),3) ;
theory_10khz = data(freq(4,:),4) ;
model_10khz = data(freq(4,:),3) ;

figure('units','normalized','outerposition',[0.05 0.05 0.9 0.9]) ;
subplot(2,2,1) ;
h = plot( range, theory_10hz, 'k-', ...
          range, model_10hz, 'ko' ) ;
set(h(1),'MarkerSize',3,'MarkerFaceColor','black') ;
set(h(2),'LineWidth', 1.25) ;
grid ;
set(gca,'Ylim',[-110 -30]) ;
ylabel('Propagation Loss (dB)') ;
xlabel('Range (km)') ;
title(sprintf('Source Depth=25 m Target Depth=200 m Freq=%0.0f Hz',p.frequency(1))) ;
legend('theory','WaveQ3D');

subplot(2,2,2) ;
h = plot( range, theory_100hz, 'k-', ...
          range, model_100hz, 'ko' ) ;
set(h(1),'MarkerSize',3,'MarkerFaceColor','black') ;
set(h(2),'LineWidth', 1.25) ;
grid ;
set(gca,'Ylim',[-110 -30]) ;
ylabel('Propagation Loss (dB)') ;
xlabel('Range (km)') ;
title(sprintf('Source Depth=25 m Target Depth=200 m Freq=%0.0f Hz',p.frequency(2))) ;
legend('theory','WaveQ3D');

subplot(2,2,3) ;
h = plot( range, theory_1khz, 'k-', ...
          range, model_1khz, 'ko' ) ;
set(h(1),'MarkerSize',3,'MarkerFaceColor','black') ;
set(h(2),'LineWidth', 1.25) ;
grid ;
set(gca,'Ylim',[-110 -30]) ;
ylabel('Propagation Loss (dB)') ;
xlabel('Range (km)') ;
title(sprintf('Source Depth=25 m Target Depth=200 m Freq=%0.0f Hz',p.frequency(3))) ;
legend('theory','WaveQ3D');

subplot(2,2,4) ;
h = plot( range, theory_10khz, 'k-', ...
          range, model_10khz, 'ko' ) ;
set(h(1),'MarkerSize',3,'MarkerFaceColor','black') ;
set(h(2),'LineWidth', 1.25) ;
grid ;
set(gca,'Ylim',[-110 -30]) ;
ylabel('Propagation Loss (dB)') ;
xlabel('Range (km)') ;
title(sprintf('Source Depth=25 m Target Depth=200 m Freq=%0.0f Hz',p.frequency(4))) ;
legend('theory','WaveQ3D');

