% profile_test.m - plot results of all profile_test sub-tests
%
clear all ; close all

% Draw the sound speed and gradient for all analytic profiles 
% models supported by USML. The models include:
%
%      - Constant Profile
%      - Linear Profile
%      - Bi-Linear Profile
%      - Munk Profile
%      - N^2 Linear Profile
%      - Catenary Profile

[data,desc] = xlsread('profile_test.csv');
N = length(data(1,:)) ;
depth = data(:,1) ;
n = 2:2:N ;
figure ;
h = plot( data(:,n), depth, 'LineWidth', 1.5 ) ;
set( h(3), 'LineStyle', 'o', 'MarkerSize', 4, 'MarkerFaceColor', [ 1.0 0.0 0.0 ] ) ;
grid ; 
xlabel('Sound Speed (m/s)')
ylabel('Depth (m)')
title('Analytic Profile Models');
set(gca,'Xlim',[1440 1560]);
% set(gca,'Ylim',[-4000 0]);
legend( desc(n), 'Location', 'EastOutside' ) ;
print -djpeg profile_test_speed.jpg

m = 3:2:N ;
figure ;
h = plot( data(:,m), depth, 'LineWidth', 1.5 ) ;
set( h(3), 'LineStyle', 'o', 'MarkerSize', 4, 'MarkerFaceColor', [ 1.0 0.0 0.0 ] ) ;
grid ; 
xlabel('Speed Gradient (m/s)')
ylabel('Depth (m)')
title('Analytic Profile Models');
set(gca,'Xlim',[-0.06 0.06]);
% set(gca,'Ylim',[-4000 0]);
legend( desc(n), 'Location', 'EastOutside' ) ;
print -djpeg profile_test_grad.jpg

% Extract Hawaii ocean temperature and salinity from World Ocean Atlas 2005.
% Compare some of the results to the interactive version at 
% UK National Physical Laboratory, "Technical Guides - 
% Speed of Sound in Sea-Water," interactive website at
% http://resource.npl.co.uk/acoustics/techguides/soundseawater/.
% 
% Generate errors if values differ by more that 1E-3 percent.

[data,desc] = xlsread('mackenzie_test.csv');
N = length(data(1,:)) ;
depth = -data(:,1) ;

figure ;

subplot(1,4,1) ;
h = plot( data(:,2), depth, 'LineWidth', 1.5 ) ;
grid ; 
set(gca,'Xlim',[0 30]);
set(gca,'Ylim',[-4000 0]);
ylabel('Depth (m)')
xlabel('Temp (degC)')

subplot(1,4,2) ;
h = plot( data(:,3), depth, 'LineWidth', 1.5 ) ;
grid ; 
set(gca,'Xlim',[34 35.1]);
set(gca,'Ylim',[-4000 0]);
xlabel('Salinity (psu)')
title('Mackenzie Model c(d,T,S)');
    
subplot(1,4,3) ;
h = plot( data(:,4), depth, 'LineWidth', 1.5 ) ;
grid ; 
set(gca,'Ylim',[-4000 0]);
xlabel('Speed (m/s)')

subplot(1,4,4) ;
h = plot( data(:,5), depth, 'LineWidth', 1.5 ) ;
grid ; 
set(gca,'Ylim',[-4000 0]);
xlabel('Gradient (1/s)')
title('June 18.5N 159.5W');

print -djpeg mackenzie_test.jpg

