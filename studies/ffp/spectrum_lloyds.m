% Transfer function for a homogenous half space.
%
% The spectrum from a point source in a homogenous half space has two
% contributions.  One is the direct path spherical wave from the source.
% The other is another spherical wave that appears as if it is coming from
% a second source that has been reflected across the interface.  This
% result is often referred to as the Method of Images.
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
%           k0          Wave number magnitude (1/m) = 2 pi f/c0
%           kr          Horizontal wave numbers (1/m)
% Outputs:  spectrum    Enviromental transfer function.

% Author: Sean Reilly (University of Rhode Island), May 2001
% 
% References: 
%   [1] F. B. Jensen, W. A. Kuperman, M. B. Porter, H. Schmidt, 
%       "Computational Ocean Acoustics", AIP Press, 1994, Chapter 2.4.2.

function spectrum = spectrum_lloyds( z, zs, k0, kr )

kz = sqrt( k0*k0 - kr.*kr ) ;
spectrum = ( exp( i*kz*abs(z-zs) ) - exp( i*kz*(z+zs) ) ) ...
         ./ (i*kz) ;

end
