%% simple_wedge
% Compute transmission loss for simple, analytic wedge where the interface
% reflection coefficients are limited to �1. Multiple receiver locations
% can be processed using vectors/matrices for range_rcv, angle_rcv,
% cross_rcv, or any combination of these receiver variables.
%
%% Inputs
%
% * wave_number = magnitude of the wave number (1/m)
% * angle_wedge = angle from bottom to surface (radians)
% * range_rcv   = slant range of receivers from the wedge apex (m)
% * angle_rcv   = angle of receivers down from the ocean surface (rad)
% * cross_rcv   = cross-slope distance of receivers relative the
%                 vertical source/origin plane (m)
% * range_src   = slant range of source from the wedge apex (m)
% * angle_src   = angle of source image down from the ocean surface (rad)
% * coherent    = compute coherent results if true, incoherent if false
%                 (optional)
% * nmax        = maximum number of bottom bounces (optional)
%
%% Outputs
%
% * intensity   = acoustic intensity at each receiver
% * eigenrays   = eigenray table for storage in *.csv file
%%
function [ intensity, eigenrays ] = simple_wedge( wave_number, angle_wedge, ...
    range_rcv, angle_rcv, cross_rcv, range_src, angle_src, coherent, nmax )

%% define coherence of solution

if ( nargin < 8 )
    coherent = true ;
end

%% define maximum number of bottom bounces

if ( nargin < 9 )
    nmax = ceil( pi / angle_wedge ) ;
end

%% allocate memory for output

d2r = pi / 180 ;
pressure = zeros(size( range_rcv .* angle_rcv .* cross_rcv )) ;
if ( nargout >= 2 ) 
    pones = ones(size(pressure));
    range_list = range_rcv .* pones ; range_list = range_list(:) ;
    angle_list = angle_rcv .* pones ; angle_list = angle_list(:) ;
    cross_list = cross_rcv .* pones ; cross_list = cross_list(:) ;
    eigenrays = [] ;
end

%% loop through contributions for each source image

for n = -nmax:nmax          % loop over number of bottom bounces
    for m = (n-1):n         % loop over number of surface bounces
            
        % compute angle to image source
        
        angle_image = 2*n*angle_wedge + (-1).^(n+m) * angle_src ;
        
        % limit source images to propagating modes
        
        if ( abs(angle_image) <= pi ) 
            
            % compute raange from source image to receiver
            
            dx = range_rcv .* cos( angle_rcv ) - range_src .* cos( angle_image ) ;
            dy = cross_rcv ;
            dz = range_rcv .* sin( angle_rcv ) - range_src .* sin( angle_image ) ;
            R = sqrt( dx.*dx + dy.*dy + dz.*dz ) ;
            theta = -asin( dz ./ R ) ;
            phi = atan2( dy, dx ) ;

            % compute complex pressure contribution
            
            contribution = (-1).^m * exp( 1i * wave_number .* R ) ./ R ;
            if ( ~coherent ) 
                contribution = abs(contribution).^2 ;
            end
            pressure = pressure + contribution ;
            
            % add contribution to eigenray list
            
            if ( nargout >= 2 )
                n_list = n .* pones ; n_list = n_list(:) ;
                m_list = m .* pones ; m_list = m_list(:) ;
                table = [ n_list m_list range_list angle_list/d2r cross_list ...
                    R(:) theta(:)/d2r phi(:)/d2r -20*log10(R(:)) ] ;
                eigenrays = [ eigenrays ; table ] ;
            end
        end
    end
end

%% convert pressure to intensity

if ( coherent ) 
    intensity = abs(pressure).^2 ;
else
    intensity = pressure ;
end

end
