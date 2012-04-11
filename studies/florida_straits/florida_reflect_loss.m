% florida_reflect_loss.m 
%
%
clear all; 
close all;

freq = 52.5 ;
angle = (0.1:0.1:89)'*pi/180 ;
cw = 1500 ;
kw = 2*pi*freq / cw ;
Zw = cw ./ sin(angle) ;

density   = [   2.40,   1.70 ] ;
speed     = [ 3000.0, 1675.0 ] ;
atten     = [    0.1,   0.01 ] ;
shear     = [ 1430.0,    0.0 ] ;
att_shear = [   0.20,    0.0 ] ;

for n=1:2
    kc = 2*pi*freq / speed(n) ;
    angc = acos( kw / kc * cos(angle) ) ;
    Zc = density(n) * speed(n) / sin(angc) ;
%     if ( shear(n) < eps )
        Ztotal = Zc ;
%     else
%         ks = 2*pi*freq / shear(n) ;
%         angs = acos( kw / ks * cos(angle) ) ;
%         Zs = density(n) * shear(n) / sin(angs) ;
%         Ztotal = Zc * cos(2*angs).^2 + Zs * sin(2*angs).^2 ;
%     end
    reflect(:,n) = abs( (Ztotal'-Zw)./(Ztotal'+Zw) ) ;
end

 figure;
 plot(angle*180/pi,reflect)
