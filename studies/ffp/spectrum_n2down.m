% Transfer function for a downward refracting N^2 linear environment.
%
%       c(z) = c0 / sqrt( 1 + a*z )   for z >= 0, a > 0
%
% Wavenumber integration cannot be performed through the poles of the
% spectrum (places where the spectrum goes to infinity). The user is
% responsible for shifting wavenumbers so that the complex integration
% contour is slightly below the poles.  The recommended shift is:
%
%           kr' = kr - i * 3 / ( R log10(e) )
%
% Inputs:   z           Target depth (meters, scalar).
%           zs          Source depth (meters, scalar)
%           k0          Wave number magnitude at surface (1/m) = 2 pi f/c(0)
%           a           Speed gradient factor (1/m, assumes a > 0)
%           kr          Horizontal wave numbers (1/m)
% Outputs:  spectrum    Enviromental transfer function.

% Author: Sean Reilly (University of Rhode Island), May 2001
% 
% References: 
%   [1] L. M. Brekhovskikh, Waves in Layered Media, 2nd Edition, 
%       Academic Press Inc., 1980, Section 54.
%   [2] F. B. Jensen, W. A. Kuperman, M. B. Porter, H. Schmidt, 
%       "Computational Ocean Acoustics", AIP Press, 1994, Chapter 2.5.1.
%   [2] M. A. Pedersen, D. F. Gordon, "Normal-Mode and Ray Theory
%       Applied to Underwater Acoustic Conditions of Extreme Downward
%       Refraction", J. Acoust. Soc. Am. 51(1), 1972.

function spectrum = spectrum_n2down( z, zs, k0, a, kr )

% frequently used terms

k02 = k0 * k0 ;
kr2 = kr .* kr ;
H = ( k02 * a )^(-1/3) ;

% compute Airy functions at ocean surface (z=0)

t0 = H*H * ( kr2 - k02 ) ;
v0 = airy( 0, t0 ) ;
Z0 = airy( 2, t0 ) + i * v0 ;

% compute Airy functions at source depth

ts = t0 - zs / H ;
vs = airy( 0, ts ) ;
Zs = airy( 2, ts ) + i * vs ;

% compute Airy functions at target depth

t = t0 - z / H ;
v = airy( 0, t ) ;
Z = airy( 2, t ) + i * v ;

% compute Green's function spectrm

if ( z <= zs )
    spectrum = H*H*H .* ( v - v0 .* ( Z ./ Z0 ) ) .* Zs ;
else
    spectrum = H*H*H .* ( vs - v0 .* ( Zs ./ Z0 ) ) .* Z ;
end

% fix places where wave number is too big for Airy functions to compute

n = find( ~isnan(spectrum) ) ;
spectrum( (max(n)+1):end ) = 0.0 ;

end
