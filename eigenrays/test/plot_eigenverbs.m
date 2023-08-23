%%
% plot_eigenverbs.m
%
function plot_eigenverbs(eigenverbs,color) 

d2r=pi/180;
lat1=15.0*pi/180.0 ;
lon1=35.0*pi/180.0 ;
angle = 0:10:360 ;  % parameter for drawing each ellipse
M = length(angle);
N = length(eigenverbs) ;

ellipse_x = zeros(M,N) ;
ellipse_y = zeros(M,N) ;
bounding_x = zeros(5,N) ;
bounding_y = zeros(5,N) ;
for n = 1:N
    
    % compute location of each eigenverb
    
    lat2 = eigenverbs(n).latitude * d2r ;
    lon2 = eigenverbs(n).longitude * d2r ;
    [range,bearing] = greatCircleDistance( ...
        lat1, lon1, lat2, lon2, 6371.01e3 ) ;
    bearing = bearing / d2r ;
    eigenverbs(n).range = range ;
    eigenverbs(n).bearing = bearing ;
    eigenverbs(n).x = range * sind(bearing) ;
    eigenverbs(n).y = range * cosd(bearing) ;
    
    % compute points on rotated ellipse
    
    point = [
        eigenverbs(n).width * cosd(angle)
        eigenverbs(n).length * sind(angle) ];
    cosA = cosd(eigenverbs(n).direction);
    sinA = sind(eigenverbs(n).direction);
    rotation = [
        cosA sinA
        -sinA cosA ] ;
    point = rotation * point ;
    ellipse_x(:,n) = eigenverbs(n).x + point(1,:) ;
    ellipse_y(:,n) = eigenverbs(n).y + point(2,:) ;
    
    % compute points on bounding box
    
    bounding_x(1,n) = eigenverbs(n).bounding_west ;
    bounding_y(1,n) = eigenverbs(n).bounding_north ;
    bounding_x(2,n) = eigenverbs(n).bounding_east ;
    bounding_y(2,n) = eigenverbs(n).bounding_north ;
    bounding_x(3,n) = eigenverbs(n).bounding_east ;
    bounding_y(3,n) = eigenverbs(n).bounding_south ;
    bounding_x(4,n) = eigenverbs(n).bounding_west ;
    bounding_y(4,n) = eigenverbs(n).bounding_south ;
    bounding_x(5,n) = eigenverbs(n).bounding_west ;
    bounding_y(5,n) = eigenverbs(n).bounding_north ;
end

figure(1);
plot( ellipse_x, ellipse_y, color, [eigenverbs.x], [eigenverbs.y], 'k.') ;
hold on;

figure(2);
plot( bounding_x, bounding_y, color, ...
    [eigenverbs.longitude], [eigenverbs.latitude], 'k.') ;
hold on;

end
