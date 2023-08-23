%%
% make_eigenverbs.m - Visualize Gaussian projections onto interface
%
% Prototype for the basic_eigenverbs test that populates the ocean bottom
% with eigenverb ellipses.  Used to visualize the interplay between range,
% bearing, length, and width.  If range, bearing, length, and width are
% defined correctly, the resulting plot should look like a collection of
% ellipses that just barely touch each other in the radial ands azimuthal
% direction.
%
clear; close all

center_lat = 45; 
center_lng = -45.0;
center_alt = -1000 ;
de_spacing = 10.0 ;
az_spacing = 10.0 ;
de = (-90+de_spacing):de_spacing:-de_spacing ;
az = 0:az_spacing:90 ;
[de_grid,az_grid] = meshgrid(de,az);
de_grid = de_grid(:)' ;
az_grid = az_grid(:)' ;

% compute location, length, and width

grazing = abs(de_grid);
horz_range = abs(center_alt) ./ tand(grazing);
slant_range = abs(center_alt) ./ sind(grazing) ;

pos_north = horz_range .* cosd(az_grid) ;
pos_east = horz_range .* sind(az_grid) ;

L = 0.5 * slant_range .* deg2rad(de_spacing) ./ sind(grazing);
W = 0.5 * slant_range .* deg2rad(az_spacing) .* cosd(grazing);

% create rotated, translated ellipse for each point

angle = 0:10:360 ;  % parameter for drawing each ellipse
N = length(pos_north) ;
M = length(angle) ;
ellipse_north = zeros(N,M) ;
ellipse_east = zeros(N,M) ;

for n=1:N
    
    point = [
        L(n) * cosd(angle)
        W(n) * sind(angle) ];
    cosA = cosd( az_grid(n) );
    sinA = sind( az_grid(n) );
    rotation = [
        cosA -sinA
        sinA cosA ] ;
    point = rotation * point ;
    ellipse_north(n,:) = pos_north(n) + point(1,:) ;
    ellipse_east(n,:) = pos_east(n) + point(2,:) ;    
end

% display each ellipse and its center

figure;
plot(pos_east,pos_north,'b.',ellipse_east',ellipse_north','b-');
xlabel('East (m)');
ylabel('North (m)');
axis('square');
grid
