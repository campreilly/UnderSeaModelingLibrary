% reflect_loss_test.m - plot results of all reflect_loss_test sub-tests
%
clear all ; close all

% Tests the version of the reflect_loss_rayleigh() constructor that takes
% a sediment type as its argument.  The parameters for each sediment type
% are defined using table 1.3 from F.B. Jensen, W.A. Kuperman, 
% M.B. Porter, H. Schmidt, "Computational Ocean Acoustics", pp. 41.
% Note that reflect_loss_rayleigh() does not implement the weak, depth 
% dependent, shear in slit, sand, and gravel.  

sheet=importdata('rayleigh_sediments.csv',',');
data = sheet.data ;
desc = sheet.colheaders ;
N = length(data(1,:)) ;
figure ;
h = plot( data(:,1), data(:,2:N), 'LineWidth', 1.5 ) ;
set( h(8), 'LineStyle', '--' ) ;
grid ; 
xlabel('Grazing Angle (deg)')
ylabel('Reflection Loss (dB)')
title('Rayleigh Reflection Loss vs. Sediment Type');
set(gca,'Xlim',[0 90]);
set(gca,'Ylim',[0 16]);
legend( desc(2:N), 'Location', 'EastOutside' ) ;
print -djpeg rayleigh_sediments.jpg

% Tests the version of the reflect_loss_rayleigh() constructor that takes
% compression and shear wave parameters as inputs.  Implements the
% four test cases (a, b, c,and d) defined in  F.B. Jensen, W.A. Kuperman, 
% M.B. Porter, H. Schmidt, "Computational Ocean Acoustics", pp. 35-49.

% case a is loss vs compression speed (d=2, a=0.5)

sheet=importdata('rayleigh_test_a.csv',',');
data = sheet.data ;
desc = sheet.colheaders ;
N = length(data(1,:)) ;
figure ;
h = plot( data(:,1), data(:,2:N), 'LineWidth', 1.5 ) ;
grid ; 
xlabel('Grazing Angle (deg)')
ylabel('Reflection Loss (dB)')
title('Rayleigh Reflection Loss (d=2, a=0.5)');
set(gca,'Xlim',[0 90]);
set(gca,'Ylim',[0 16]);
legend( desc(2:N), 'Location', 'EastOutside' ) ;
print -djpeg rayleigh_test_a.jpg

% case b is loss vs attenuation (d=2, cp=1600)

sheet=importdata('rayleigh_test_b.csv',',');
data = sheet.data ;
desc = sheet.colheaders ;
N = length(data(1,:)) ;
figure ;
h = plot( data(:,1), data(:,2:N), 'LineWidth', 1.5 ) ;
grid ; 
xlabel('Grazing Angle (deg)')
ylabel('Reflection Loss (dB)')
title('Rayleigh Reflection Loss (d=2, cp=1600)');
set(gca,'Xlim',[0 90]);
set(gca,'Ylim',[0 16]);
legend( desc(2:N), 'Location', 'EastOutside' ) ;
print -djpeg rayleigh_test_b.jpg

% case c is loss vs density (cp=1600, a=0.5)

sheet=importdata('rayleigh_test_c.csv',',');
data = sheet.data ;
desc = sheet.colheaders ;
N = length(data(1,:)) ;
figure ;
h = plot( data(:,1), data(:,2:N), 'LineWidth', 1.5 ) ;
grid ; 
xlabel('Grazing Angle (deg)')
ylabel('Reflection Loss (dB)')
title('Rayleigh Reflection Loss (cp=1600, a=0.5)');
set(gca,'Xlim',[0 90]);
set(gca,'Ylim',[0 16]);
legend( desc(2:N), 'Location', 'EastOutside' ) ;
print -djpeg rayleigh_test_c.jpg

% case d is loss vs shear speed (d=2, cp=1600, a=0.5)

sheet=importdata('rayleigh_test_d.csv',',');
data = sheet.data ;
desc = sheet.colheaders ;
N = length(data(1,:)) ;
figure ;
h = plot( data(:,1), data(:,2:N), 'LineWidth', 1.5 ) ;
grid ; 
xlabel('Grazing Angle (deg)')
ylabel('Reflection Loss (dB)')
title('Rayleigh Reflection Loss (d=2, cp=1600, a=0.5)');
set(gca,'Xlim',[0 90]);
set(gca,'Ylim',[0 16]);
legend( desc(2:N), 'Location', 'EastOutside' ) ;
print -djpeg rayleigh_test_d.jpg
