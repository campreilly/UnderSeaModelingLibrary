% Generate a tangent spaced sequence of rays

clear all ; close all ;

first   = -90.0 ;
last    = 90.0 ;
spread  = 6.0 ;
center  = 0.0 ;
num     = 45.0 ;
range   = (0:1e3:12e3)' ;

first_ang = first-center ;
last_ang = last-center ;
scale = (last_ang - first_ang) / (num - 1)
n=1:num ;
x = first_ang + scale * (n-1) ;
angle = center + x ;

a = angle * pi / 180 ;
x = range * cos(a) ;
y = -75.0 + range * sin(a) ;
figure ;
subplot(2,1,1);
plot(x/1e3,y,'k-');
axis([0 10 -200 0]);
ylabel('Depth (m)');
title(sprintf('%d uniformly spaced rays',num));

first_ang = atan( (first-center)/spread ) ;
last_ang = atan( (last-center)/spread ) ;
scale = (last_ang - first_ang) / (num - 1) ;
n=1:num ;
x = first_ang + scale * (n-1) ;
angle = center + tan(x)*spread 

a = angle * pi / 180 ;
x = range * cos(a) ;
y = -75.0 + range * sin(a) ;
subplot(2,1,2);
plot(x/1e3,y,'k-');
axis([0 10 -200 0]);
xlabel('Range (km)');
ylabel('Depth (m)');
title(sprintf('%d tangent spaced rays',num));
print -deps plot_seq_rayfan

figure;
polar( angle*pi/180, ones(size(angle)), 'o' );
fprintf('std = %f deg, min diff = %f deg\n',std(angle),min(diff(angle)));
