%%
% Wave number integration for a specific image source (equation 28) in the
% Deane/Buckingham March 1993 paper. Limits the evaluation to 2-D
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
% Loops over targets so that integration can be performed on a
% target-by-target basis.
%
%   pressure = wedge_integ_2d( num_surface, num_bottom, ...
%       wave_number, range, zeta )
%
%   pressure    = matrix of complex pressures (one for each target).
%   num_surface = number of surface bounces for current source
%   num_bottom  = number of bottom bounces for current source
%   wave_number = magnitude of the acoustic wave number (scalar)
%   range       = matrix of target ranges from source
%   zeta        = matrix of target depression/elevation angles relative to source
%
function pressure = wedge_integ_2d( num_surface, num_bottom, ...
    wave_number, range, zeta )

    [Nrows,Ncols] = size(range) ;
    pressure = zeros(Nrows,Ncols) ;
    for row = 1:Nrows
        for col = 1:Ncols
            % Error: complex portion of spectrum goes to Inf for large X!!!
            % Should be decaying.
            %
            % for axis = 0:1  % 0 = real axis, 1 = imag axis
            for axis = 0	% for debugging only!!!

                if ( axis == 0 )
                    % integrate along a real contour from 0 to pi/2 to represent the
                    % contributions from traveling waves
                    N = 3000 ;
                    theta = (0:1/(N-1):1) * pi/2 ;
                    x = theta ;         % plotting is for debugging only!!!
                else
                    % integrate along an imaginary contour from pi/2 to pi/2 - iX
                    % to represent the contribution from evanescent waves.
                    N = 500 ;
                    theta = pi/2 - 1i * (0:1/(N-1):1) * pi/4 ;
                    x = imag(theta) ;   % plotting is for debugging only!!!
                end

                spectrum = wedge_spectrum_2d( ...
                    num_surface, num_bottom, wave_number, theta, ...
                    range(row,col), zeta(row,col) ) ;
                pressure(row,col) = pressure(row,col) ...
                    + simpson( theta(2)-theta(1), spectrum ) ;

                figure;
                plot( x, abs(spectrum) ) ;  % plotting is for debugging only!!!
            end
        end
    end
    
end
