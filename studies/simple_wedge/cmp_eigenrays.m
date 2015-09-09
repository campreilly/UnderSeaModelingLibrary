%% cmp_eigenrays
% Compare eigenrays from WaveQ3D to analymodel.  Used as a debugging
% tool during scenario development.
%
clear ; close all
deg2km = ( 1852.0 * 60.0 ) / 1e3 ;

[ proploss, eigenrays ] = load_proploss('simple_wedge_eigenray.nc');

r = 0 ;
for n=1:length(eigenrays)
    rays = eigenrays(n) ;
    for m=1:length(rays.intensity)
        r = r + 1 ;
        table(r,1) = double(rays.bottom(m)) ;   % number of bottom bounces
        table(r,2) = double(rays.surface(m)) ;  % number of surface bounces
        table(r,3) = 0.0 ;              % down-range distance from apex
        table(r,4) = 0.0 ;              % angle down from surface
        table(r,5) = n * 1000.0 ;       % distance across slope
        table(r,6) = rays.travel_time(m) * 1500 ; % slant range to source
        table(r,7) = rays.target_de(m) ;    % theta angle
        table(r,8) = rays.target_az(m) ;    % phi angle
        table(r,9) = -rays.intensity(m) ;   % transmission loss (dB)
    end
end

csvwrite('cmp_eigenrays.csv',table) ;