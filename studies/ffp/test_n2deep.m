% Demonstrate the FFP for N^2 Linear using the parameters defined 
% for Figure 3.7 in Jensen, Kuperman, et. al., Computational 
% Ocean Acoustics, section 3.3.

clear all
close all

% define range and wavenumber axes

N = 8192 ;                      % number of points to use in FFP
kmin = 6.5 ;                    % minimum wavenumber for integration (1/m)
kmax = 12.5 ;                   % maximum wavenumber for integration (1/m)
rmin = 3000 ;                   % minimum range for solution (meters)
rmax = 3100 ;                   % maximum range to plot in solution (meters)

dk = (kmax-kmin)/N ;            % wavenumber increment (1/m)
R = 2*pi/dk ;                   % range window extent (meters)
offset = 3/(R*log10(exp(1))) ;  % move contour away from poles
kr = kmin + (0:N-1) * dk - i*offset ;  % shifted wavenumber axis

% define other scenario parameters

z = 800.0 ;                     % target depths (meters)
zs = 1000.0 ;                   % source depth (meters)
freq = 2000 ;                   % source frequency (Hz)
c0 = 1550 ;                     % speed of sound at ocean surface (m/s)
a = 2.4 / c0 ;                  % speed gradient factor (1/m)

% compute wavenumber spectrum and plot it

k0 = 2*pi*freq/c0 ;             % wave number magnitude at surface
spectrum = spectrum_n2down( z, zs, k0, a, kr ) ;

figure;
plot( real(kr), abs(spectrum) ) ;
grid ;
xlabel('Horizontal wavenumber (1/m)')
ylabel('Magnitude');
header = sprintf('N^2 Linear: c0=%.1f a*c0=%.1f z=%.1f zs=%.1f f=%.1f Hz',...
    c0,a*c0,z,zs,freq) ;
title(header) ;

% integrate the wavenumber spectrum and plot it

[loss,ranges] = fast_field_program(spectrum,kr,z,rmin) ;
n = ranges <= rmax ;               % trim to desired range scale
ranges = ranges(n) ;
loss = 20*log10( abs(loss(n)) ) ;  % convert to dB scale

figure;
plot( ranges, loss ) ;
grid ;
xlabel('Range (m)')
ylabel('Transmission loss (dB)');
title(header) ;
set(gca,'YLim',[-80 -40])
