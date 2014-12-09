close all ; clear all ;

[data, desc] = xlsread('scattering_lambert_test.csv') ;

angle = data(:,1)*180.0/pi ;
tl = 10.0*log10( data(:,3) ) ;

bss = 10^(-2.7) ;
a = 0:90 ;
I = a * pi/180.0 ;
S = pi/4.0 ;
lambert = bss * sin(I) .* sin(S) ;
lambert = 10*log10( lambert ) ;

figure ;
plot( angle, tl, 'b.-', ...
      a, lambert, 'rd-' ) ;
xlabel('Scattering Angle (deg)') ;
ylabel('Scattering Strength (dB)') ;
legend('waveq3d','analytic','location','northeast') ;
title( ...
  sprintf('Scattering Strength using Lambert\''s law\nConstant scattering angle of 45 deg') ) ;
grid on ;