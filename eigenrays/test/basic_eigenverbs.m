%%
% basic_eigenverbs.m
%
clear; close all
eigenverbs = load_eigenverbs('basic_eigenverbs.nc');
plot_eigenverbs(eigenverbs,'b') ;
found = load_eigenverbs('find_eigenverbs.nc');
plot_eigenverbs(found,'r') ;
plot_eigenverbs(eigenverbs(1),'g') ;

figure(1);
xlabel('East (m)');
ylabel('North (m)');
axis('square');
grid
axis([-50 550 -50 550]);
hold off

figure(2);
xlabel('East (deg)');
ylabel('North (deg)');
axis('square');
grid
axis([34.999 35.005 14.999 15.005]);
hold off
