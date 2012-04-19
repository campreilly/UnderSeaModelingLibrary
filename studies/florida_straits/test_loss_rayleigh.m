%%
% reflect_loss_test - test the reflection loss model by re-creating
% Figure 1.2.3 from Jensen, Kuperman, et. al., Computational Ocean Acoustics,
% 2nd edition, p. 44.

clear all ;
close all ;

degrees = [1e-6 1:90] ;
angles = degrees/90*pi/2 ;
speed_bottom = [ 1550 1600 1800 ] ;
att_bottom = [ 0.0, 0.5, 1.0 ] ;
density = [ 1500 2000 2500 ] ;
speed_shear = [ 0 200 400 600 ] ;

for n=1:length(speed_bottom)
    loss_speed(n,:) = reflect_loss_rayleigh( angles, 2000, speed_bottom(n), 0.5, 0, 0 ) ;
end
for n=1:length(att_bottom)
    loss_att(n,:) = reflect_loss_rayleigh( angles, 2000, 1600, att_bottom(n), 0, 0 ) ;
end
for n=1:length(density)
    loss_density(n,:) = reflect_loss_rayleigh( angles, density(n), 1600, 0.5, 0, 0 ) ;
end
for n=1:length(speed_shear)
    loss_shear(n,:) = reflect_loss_rayleigh( angles, 2000, 1600, 0.0, speed_shear(n), 0 ) ;
end

figure;
subplot(2,2,1);
plot( degrees, loss_speed ) ; grid ;
xlabel('Grazing angle (deg)');
ylabel('Bottom loss (dB)');
title('Compresional Speed (m/s)');
axis([0 90 0 15]);
set(gca,'XTick',0:15:90);
set(gca,'YTick',0:2.5:15);
legend('1550','1600','1800','Location','SouthEast');

subplot(2,2,2);
plot( degrees, loss_att ) ; grid ;
xlabel('Grazing angle (deg)');
ylabel('Bottom loss (dB)');
title('Compresional Att (dB/wleng)');
axis([0 90 0 15]);
set(gca,'XTick',0:15:90);
set(gca,'YTick',0:2.5:15);
legend('0.0','0.5','1.0','Location','SouthEast');

subplot(2,2,3);
plot( degrees, loss_density ) ; grid ;
xlabel('Grazing angle (deg)');
ylabel('Bottom loss (dB)');
set(gca,'XTick',0:15:90);
title('Density (kg/m^3)');
axis([0 90 0 15]);
set(gca,'XTick',0:15:90);
set(gca,'YTick',0:2.5:15);
legend('1500','2000','2500','Location','SouthEast');

subplot(2,2,4);
plot( degrees, loss_shear ) ; grid ;
xlabel('Grazing angle (deg)');
ylabel('Bottom loss (dB)');
title('Shear Speed (m/s)');
axis([0 90 0 15]);
set(gca,'XTick',0:15:90);
set(gca,'YTick',0:2.5:15);
legend('0','200','400','600','Location','SouthEast');
