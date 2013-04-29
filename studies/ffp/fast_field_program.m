% Fast Field Program (FFP) 
% 
% This algorithm performs wavenumber integration on a range-independent
% environmental transfer function using an inverse FFT.
%
% The wavenumber and range axes are treated as an FFT conjugate pair. This
% means that the extent and spacing of the range window are predefined by
% the extent and spacing of the wavenumber window.  
%
%       R = 2 pi / dk, dr = 2 pi / K, where
%       R, K = extents of the range and wavenumber windows.
%       dr, dk = spacing in the range and wavenumber windows.
% 
% The FFP algorithm supports cases where range and wavenumber windows do
% not start at zero.  Ripples can be introduced to the result of minimum
% wavenumber is not at a minimum of the wave spectrum amplitude.
%
% Wavenumber integration cannot be performed through the poles of the
% spectrum (places where the spectrum goes to infinity). The user is
% responsible for shifting wavenumbers so that the complex integration
% contour is slightly below the poles.  The recommended shift is:
%
%           kr' = kr - i * 3 / ( R log10(e) )
%
% Inputs:   spectrum    Enviromental transfer function as a function of 
%                       wavenumber (columns) and depth (rows).
%                       Length should be a power of 2 to maximize the
%                       efficiency of the IFFT.
%           wavenumbers Horizontal wavennumbers associated with spectrum.
%           depths      Depths associated with spectrum.
%                       The can be a vector or a scalar.
%           rmin        Minimum range for result (optional, default=0).
% Ouptuts:  loss        Propagation loss in the form of a complex pressure
%                       as a function of range (columns) and depth (rows).
%           ranges      Ranges associated with transmission loss.

% Author: Sean Reilly (University of Rhode Island), May 2001
% 
% References: 
%   [1] F. R. DiNapoli, R. L. Deavenport, "Theoretical and numerical 
%       Green's function field solution in a plane multilayered medium", 
%       J. Acoust. Soc. Am. 67(1), Jan 1980.
%   [2] F. B. Jensen, W. A. Kuperman, M. B. Porter, H. Schmidt, 
%       "Computational Ocean Acoustics", AIP Press, 1994, Chapter 4.

function [loss,ranges] = fast_field_program(spectrum,wavenumbers,depths,rmin)
if ( nargin < 4 ), rmin = 0.0; end

% build FFT conjugant pairs for wavenumbers and ranges
% (ref [2] section 4.5.4)

N = length(wavenumbers) ;           % number of points in FFT
kmin = real(wavenumbers(1)) ;       % minimum wave number
dk = real(wavenumbers(2))-kmin ;    % wavenumber increment

dr = 2*pi / (2*N*dk);               % range inc (# wavenums is doubled)
ranges = rmin + (0:N-1) * dr ;      % range axis

% use IFFT to compute TL vs. range at each depth 
% double the number of IFFT points to avoid aliasing
% (ref [1] eqn. 14, ref [2] eqn. 4.100).

n = 1:N ;
loss = -300*ones( length(depths), N ) ;
for d = 1:length(depths)
    F = spectrum(d,:) .* exp(i*rmin*(wavenumbers-kmin)) .* sqrt(wavenumbers) ;
    F = ifft( F, 2*N )*(2*N) ;      % normalized IFFT
    loss(d,:) = dk * sqrt(1/(pi*i)) * exp(i*kmin*ranges) ./ sqrt(ranges) .* F(n) ;
end

end
