%
% reflection_test.m - plot results of all reflection_test sub-tests
%
clear all ; close all

% propagate over a 1000 meter bottom depth
% use an initial direction that results in a 0.2 deg hop distance

[data,desc] = xlsread('reflect_flat_test.csv');
dt = ( data(2,1) - data(1,1) ) * 1000.0 ;

figure ;
h = plot( data(:,2), data(:,4), 'k-', 'LineWidth', 1.25 ) ;
grid ;
ylabel('Depth (m)')
xlabel('Latitude (deg)')
print -deps reflect_flat_test

% propagate up a flat slope
% creates a 2 degree increase in grazing angle for each reflection

[data,desc] = xlsread('reflect_slope_test.csv');
dt = ( data(2,1) - data(1,1) ) * 1000.0 ;

figure ;
h = plot( data(:,2), data(:,4), 'k-', 'LineWidth', 1.25 ) ;
grid ;
patch([45.1 45.4 45.4],[-1000 -425 -1000],[0.9 0.9 0.9])
ylabel('Depth (m)')
xlabel('Latitude (deg)')
print -deps reflect_slope_test

% propagate over a gridded bottom

[data,desc] = xlsread('reflect_grid_test.csv');
dt = ( data(2,1) - data(1,1) ) * 1000.0 ;

figure ;
bathymetry = load_bathymetry( '../../data/bathymetry/med_ocean.nc' ) ;
n = find( abs(bathymetry.longitude-15.8) < 1.0 ) ;
m = find( abs(bathymetry.latitude-36.1) < 1.0 ) ;
[C,h] = contour(bathymetry.longitude(n),bathymetry.latitude(m),...
    -bathymetry.altitude(m,n),500:500:4000, ...
    'LineColor','black','LineStyle',':');
hold on

plot( data(:,3), data(:,2), 'k-', 'LineWidth', 1.5 ) ;
grid ;
ylabel('Latitude (deg)')
xlabel('Longitude (deg)')
axis([15.4 16.0 35.8 36.4])
clabel(C,h)
hold on

delta=diff(data(:,4));
n=find( delta(1:(end-1)) .* delta(2:end) < 0 ) ;
m = n(1:2:end) ;
plot( data(m,3), data(m,2), 'ko', 'LineWidth', 1.5, 'MarkerFaceColor', 'black' ) ;
m = n(2:2:end) ;
plot( data(m,3), data(m,2), 'ko', 'LineWidth', 1.5 ) ;
hold off

print -deps reflect_grid_malta
