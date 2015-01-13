%% Plot results produced from proploss_lloyds_range_freq
close all ;
clear all ;

[p, e] = load_netcdf_proploss('proploss_lloyds_range_freq.nc') ;
size = length(p.latitude) ;     % Number of targets
[data, desc] = xlsread('proploss_lloyds_range_freq.csv') ;
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

