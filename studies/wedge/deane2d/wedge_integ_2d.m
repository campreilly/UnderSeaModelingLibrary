%%
% Wave number integration for a specific image source (equation 28) in the
% Deane/Buckingham March 1993 paper.  Limits the evaluation to 2-D
% contributions, those in the plane of the source. Simpson's rule is used
% to evaluate the complex contour integral along two paths.
%
%    - The first path varies the theta coordinate (wavenumber D/E) 
%      along the real axis from 0 to pi/2.  This represents the 
%      oscillitory component of the pressure field.
%
%    - The second path varies the theta coordinate (wavenumber D/E) 
%      along the imaginary direction from pi/2 to pi/2 - i Inf.  
%      This represents the evanescent (decaying) component of the 
%      pressure field.
%
%   pressure = wedge_integ_2d( num_surface, num_bottom, ...
%       wave_number, range, zeta )
%
%   pressure    = complex pressure for one image source
%   num_surface = number of surface bounces for current source
%   num_bottom  = number of bottom bounces for current source
%   wave_number = magnitude of the acoustic wave number (scalar)
%   range       = target range from source
%   zeta        = target depression/elevation angle relative to source
%
% Relies on global variable for the bottom properties of wedge_angle,
% density, speed, atten, speed_shear, atten_shear.
%
function pressure = wedge_integ_2d( num_surface, num_bottom, ...
    wave_number, range, zeta )

    global wedge_angle density speed atten speed_shear atten_shear
    
    % integrate along a real contour from 0 to pi/2 to represent the
    % contributions from traveling waves

    N = 3000 ;
    theta = (0:1/(N-1):1) * pi/2 ;
    spectrum = wedge_spectrum_2d( num_surface, num_bottom, ...
        wave_number, theta, range, zeta ) ;
    pressure = simpson( theta(2)-theta(1), spectrum ) ;
    
    figure; 
    plot( theta, abs(spectrum) ) ;
    
    % integrate along an imaginary contour from pi/2 to pi/2 - iX 
    % to represent the contribution from evanescent waves.

    N = 500 ;
    theta = pi/2 - 1i * (0:1/(N-1):1) * pi/4 ;
    spectrum = wedge_spectrum_2d( num_surface, num_bottom, ...
        wave_number, theta, range, zeta ) ;
    pressure = pressure + simpson( theta(2)-theta(1), spectrum ) ;
    
    figure; 
    plot( theta, abs(spectrum) ) ;
    
end
