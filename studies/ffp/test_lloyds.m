% Demonstrate the FFP for Lloyd's Mirror using the parameters defined 
% for Figure 2.7 in Jensen, Kuperman, et. al., Computational 
% Ocean Acoustics, section 4.5.4.

clear all
close all

% define range and wavenumber axes

N = 8192 ;                      % number of points to use in FFP
kmin = 0 ;                      % minimum wavenumber for integration (1/m)
kmax = pi ;                     % maximum wavenumber for integration (1/m)
rmin = 0 ;                      % minimum range for solution (meters)
rmax = 500 ;                    % maximum range to plot in solution (meters)

dk = (kmax-kmin)/N ;            % wavenumber increment (1/m)
R = 2*pi/dk ;                   % range window extent (meters)
offset = 3/(R*log10(exp(1))) ;  % move contour away from poles
kr = kmin + (0:N-1) * dk - i*offset ;  % shifted wavenumber axis

% define other scenario parameters

zs = 25.0 ;                     % source depth (meters)
freq = 150 ;                    % source frequency (Hz)
c0 = 1500 ;                     % speed of sound at ocean surface (m/s)

% compute the two spectra show in ref [1] Figure 2.7

k0 = 2*pi*freq/c0 ;             % wave number magnitude
lamda = c0 / freq ;

z1 = zs + lamda/10 ;
spectrum1 = spectrum_lloyds( z1, zs, k0, kr ) ;

z2 = zs + 2*lamda ;
spectrum2 = spectrum_lloyds( z2, zs, k0, kr ) ;

figure;
plot( real(kr), abs(spectrum1), 'b-', real(kr), abs(spectrum2), 'r:' ) ;
grid ;
xlabel('Horizontal wavenumber (1/m)')
ylabel('Magnitude');
header = sprintf('Lloyd''s Mirror: c0=%.1f zs=%.1f f=%.1f Hz',c0,zs,freq) ;
title(header) ;
legend( 'z=lamda/10', 'z=2*lamda', 'Location', 'NorthWest' ) ;
set(gca,'XLim',[0 1.0])
set(gca,'YLim',[0 50.0])
drawnow ;

% compute the wavenumber spectrum at multiple depths and integrate it.

depths = 2:2:500 ;          % target depths (meters)
for n=1:length(depths)
    z = depths(n) ;
    spectrum(n,:) = spectrum_lloyds( z, zs, k0, kr ) ;
end
[loss,ranges] = fast_field_program(spectrum,kr,depths,rmin) ;

n = ranges <= rmax ;        % trim to desired range scale
ranges = ranges(n) ;
loss = 20*log10( abs(loss(:,n)) ) ;  % convert to dB scale

figure;
surf( ranges, -depths, loss, 'EdgeColor', 'none', 'FaceColor', 'interp' )
set(gca,'CLim',[-60 -30]);
view([0 90]);
xlabel('Range (m)')
ylabel('Depth (m)');
title(header) ;
colorbar;
