%%
% Tests reflection.m using the scenario defined in Figure 1.22 of
% Jensen, Kuperman, Porter, Schmidt, Computational Ocean Acoustics,
% 1st Edition, 1994.  Computes reflection loss as a function of
% sound speed, attenuation, density, and shear speed.
%
function reflection_test
    close all;
    vs_sound_speed ;
    vs_attenution ;
    vs_density ;
    vs_shear ;
end

%%
% Compute reflection loss as a function of sound speed.
%
function vs_sound_speed
    density = 2 ;
    atten = 0.5 ;
    [speed,angle] = meshgrid([1550 1600 1800]/1500, 0:90 ) ;
    R = reflection( (90-angle)*pi/180, density, speed, atten ) ;
    loss = -20.0 * log10( abs(R) ) ;

    figure ;
    plot( angle, loss ) ;
    annotate ;
    legend('c=1550','c=1600','c=1800','Location','SouthEast');
end

%%
% Compute reflection loss as a function of attenution
%
function vs_attenution
    density = 2 ;
    speed = 1600/1500 ;
    [atten,angle] = meshgrid([0 0.5 1.0], 0:90 ) ;
    R = reflection( (90-angle)*pi/180, density, speed, atten ) ;
    loss = -20.0 * log10( abs(R) ) ;

    figure ;
    plot( angle, loss ) ;
    annotate ;
    legend('a=0.0','a=0.5','a=1.0','Location','SouthEast');
end

%%
% Compute reflection loss as a function of density.
%
function vs_density
    speed = 1600/1500 ;
    atten = 0.5 ;
    [density,angle] = meshgrid([1500 2000 2500]/1000, 0:90 ) ;
    R = reflection( (90-angle)*pi/180, density, speed, atten ) ;
    loss = -20.0 * log10( abs(R) ) ;

    figure ;
    plot( angle, loss ) ;
    annotate ;
    legend('d=1500','d=2000','d=2500','Location','SouthEast');
end

%%
% Compute reflection loss as a function of shear speed.
%
function vs_shear
    density = 2 ;
    speed = 1600/1500 ;
    atten = 0.0 ;
    [speed_shear,angle] = meshgrid([0 200 400 600]/1500, 0:90 ) ;
    atten_shear = 0.0 ;
    R = reflection( (90-angle)*pi/180, density, speed, atten, speed_shear, atten_shear ) ;
    loss = -20.0 * log10( abs(R) ) ;

    figure ;
    plot( angle, loss ) ;
    annotate ;
    legend('cs=0','cs=200','cs=400','cs=600','Location','SouthEast');
end

%%
% Annotate the plot.
%
function annotate
    grid ;
    xlabel('Grazing Angle (deg)');
    ylabel('Bottom Loss (dB)');
    axis([0 90 0 14]);
end
