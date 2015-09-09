%% simple_flat
% Varient of simple_wedge calculation for a flat bottom.
% Uses arguments similar to those of simple_wedge to make comparisons
% easier.
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
%
%% Outputs
%
% * pressure    = complex pressure at each receiver
%
function pressure = simple_flat( wave_number, angle_wedge, ...
    range_rcv, angle_rcv, cross_rcv, range_src, angle_src  )

% convert wedge coordinates to flat bottom scenario

x_rcv = range_rcv .* cos( angle_rcv ) ;
z_rcv = range_rcv .* sin( angle_rcv ) ;
x_src = range_src .* cos( angle_src ) ;
z_src = range_src .* sin( angle_src ) ;
depth = x_src .* tan( angle_wedge ) ;

% compute pressure using method of images

pressure = zeros(size( range_rcv .* angle_rcv .* cross_rcv )) ;
nmax = 1000 ;
for n = -nmax:nmax          % loop over number of bottom bounces
    for m = (n-1):n         % loop over number of surface bounces
        
        % compute depth of image source
        
        z_image = 2*n*depth + (-1)^(n+m) * z_src ;
        
        % compute raange from source image to receiver

        dx = x_src - x_rcv ;
        dy = cross_rcv ;
        dz = z_image - z_rcv ;
        R = sqrt( dx.*dx + dy.*dy + dz.*dz ) ;

        % compute complex pressure contribution

        pressure = pressure + ...
            (-1)^m * exp( 1i * wave_number .* R ) ./ R ;
    end
end

end
