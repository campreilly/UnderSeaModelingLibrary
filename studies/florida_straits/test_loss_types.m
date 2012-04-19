%%
% reflect_loss_type - test the reflection loss model by re-creating
% Figure 1.2.3 from Jensen, Kuperman, et. al., Computational Ocean Acoustics,
% 2nd edition, p. 44.

clear all ;
close all ;

degrees = [1e-6 1:90] ;
angles = degrees/90*pi/2 ;
speed_bottom    = [ 1500 1575 1650 1800 1950 2400 3000 5250 ] ;
att_bottom      = [ 0.2 1.0 0.8 0.6 0.4 0.2 0.1 0.1 ] ;
speed_shear     = [ 0 0 0 0 600 1000 1500 2500 ] ;
att_shear       = [ 1.0 1.5 2.5 1.5 1.0 0.5 0.2 0.2 ] ;
density         = [ 1.5 1.7 1.9 2.0 2.1 2.2 2.4 2.7 ]*1000 ;

for n=1:length(density)
    loss(n,:) = reflect_loss_rayleigh( angles, ...
        density(n), speed_bottom(n), att_bottom(n), ...
        speed_shear(n), att_shear(n) ) ;
end

figure;
plot( degrees, loss ) ; grid ;
xlabel('Grazing angle (deg)');
ylabel('Bottom loss (dB)');
axis([0 90 0 20]);
set(gca,'XTick',0:15:90);
set(gca,'YTick',0:2.5:20);
legend('Clay','Silt','Sand','Gravel','Moraine','Chalk',...
       'Limestone','Basalt','Location','NorthEast');
