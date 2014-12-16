% wave_height_pierson_test.m
%
% Plot data output from Pierson and Moskowitz model for computing 
% wave height from wind speed. Uses data hard copied from
% boundary_test/wave_height_pierson_test screen outputs.
% Compare to significant wave height plot from 
% http://www.wikiwaves.org/Ocean-Wave_Spectra.

clear; close all;

a = [
    0 	0
    1 	0.022449
    2 	0.0897959
    3 	0.202041
    4 	0.359184
    5 	0.561224
    6 	0.808163
    7 	1.1
    8 	1.43673
    9 	1.81837
    10 	2.2449
    11 	2.71633
    12 	3.23265
    13 	3.79388
    14 	4.4
    15 	5.05102
    16 	5.74694
    17 	6.48776
    18 	7.27347
    19 	8.10408
    20 	8.97959
    21 	9.9
    22 	10.8653
    23 	11.8755
    24 	12.9306
    25 	14.0306
];

figure;
plot( a(:,1), a(:,2) ); grid
axis([0 25 0 20]);
xlabel('Wind Speed U_{10} (m/s)')
ylabel('Significant Wave Height (m)');

set(gca,'XTick',0:5:25); set(gca,'XMinorTick','on');
set(gca,'YTick',0:5:20); set(gca,'YMinorTick','on');




