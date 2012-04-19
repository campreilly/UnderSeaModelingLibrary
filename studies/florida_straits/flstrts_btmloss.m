%%
% flstrts_btmloss.m - Test the reflection loss parameter selections
% by re-creating Figure 3 from Ballad paper.  Note that the compressional
% attenuation value of 0.8 dB/lambda for the sandy bottom is much higher 
% than the 0.01 value cited by Ballard.
%
% M.S. Ballard, Modeling three-dimensional propagation in a
% continental shelf environment, J. Acoust. Soc. Am. 131 (3), March 2012.
%
clear all ;
close all ;

degrees = [1e-6 1:90] ;
angles = degrees/90*pi/2 ;
speed_bottom    = [ 3000 1676 ] ;
att_bottom      = [ 0.10 0.8 ] ;
speed_shear     = [ 1430    0 ] ;
att_shear       = [ 0.20    0 ] ;
density         = [ 2.40 1.70 ]*1000 ;

for n=1:length(density)
    loss(n,:) = reflect_loss_rayleigh( angles, ...
        density(n), speed_bottom(n), att_bottom(n), ...
        speed_shear(n), att_shear(n) ) ;
end

figure;
plot( degrees, loss ) ; grid ;
xlabel('Grazing angle (deg)');
ylabel('Bottom loss (dB)');
axis([0 90 0 15]);
set(gca,'XTick',0:10:90);
set(gca,'YTick',0:2.5:15);
legend('Limestone','Carbonate Sands','Location','NorthEast');

print -dpng flstrts_btmloss


