%%
% Test Matlab versions of Rayleigh reflection loss from ocean bottom.  
% Reproduces test cases a, b, c, and d illustrated in Jensen, Kuperman, 
% Porter, Schmidt, "Computational Ocean Acoustics", Table 1.4 and 
% Figure 1.22, Chapter 1.6.1, pp 46-47.
%
function test_rayleigh_loss
close all;

rayleigh_test_a() ;
rayleigh_test_b() ;
rayleigh_test_c() ;
rayleigh_test_d() ;

end

%%
% Computes reflection loss as function of angle and sound speed.
%
function rayleigh_test_a

    % compute reflection loss
    
    angle = 0:90 ;
    density = 2.0 ;
    speed = [ 1550, 1600, 1800 ] / 1500 ;
    atten = 0.5 ;
    [ang,spd] = meshgrid( angle*pi/180, speed ) ;
    loss = rayleigh_loss( ang, density, spd, atten ) ;
    dB = -10*log10(abs(loss)) ;
    
    % plot results
    
    figure ;
    plot( angle, dB, 'LineWidth', 1.5 ) ;
    add_labels() ;
    title('Rayleigh Reflection Loss (\rho=2.0, \alpha=0.5, c_s=0)');
    legend( 'c_p=1550', 'c_p=1600', 'c_p=1800' ) ;
    
end

%%
% Compute reflection loss as function of grazing angle and attenuation.
%
function rayleigh_test_b

    % compute reflection loss
    
    angle = 0:90 ;
    density = 2.0 ;
    speed = 1600/1500 ;
    atten = [ 1.0, 0.5, 0.0 ] ;
    [ang,att] = meshgrid( angle*pi/180, atten ) ;
    loss = rayleigh_loss( ang, density, speed, att ) ;
    dB = -10*log10(abs(loss)) ;
    
    % plot results
    
    figure ;
    plot( angle, dB, 'LineWidth', 1.5 ) ;
    add_labels() ;
    title('Rayleigh Reflection Loss (\rho=2.0, c_p=1600, c_s=0)');
    legend( '\alpha=1.0', '\alpha=0.5', '\alpha=0.0' ) ;
end

%%
% Compute reflection loss as function of grazing angle and density.
%
function rayleigh_test_c

    % compute reflection loss
    
    angle = 0:90 ;
    density = [ 1.5, 2.0, 2.5 ] ;
    speed = 1600 / 1500 ;
    atten = 0.5 ;
    [ang,den] = meshgrid( angle*pi/180, density ) ;
    loss = rayleigh_loss( ang, den, speed, atten ) ;
    dB = -10*log10(abs(loss)) ;
    
    % plot results
    
    figure ;
    plot( angle, dB, 'LineWidth', 1.5 ) ;
    add_labels() ;
    title('Rayleigh Reflection Loss (c_p=1600, \alpha=0.5, c_s=0)');
    legend( '\rho=1.5', '\rho=2.0', '\rho=2.5' ) ;
end

%%
% Computes reflection loss as function of angle and shear speed.
%
function rayleigh_test_d

    % compute reflection loss
    
    angle = 0:90 ;
    density = 2.0 ;
    speed = 1600 / 1500 ;
    atten = 0.0 ;
    shear = [ 0.0, 200.0, 400.0, 600.0 ] / 1500.0 ;
    [ang,sh] = meshgrid( angle*pi/180, shear ) ;
    loss = rayleigh_loss( ang, density, speed, atten, sh ) ;
    dB = -10*log10(abs(loss)) ;
    
    % plot results
    
    figure ;
    plot( angle, dB, 'LineWidth', 1.5 ) ;
    add_labels() ;
    title('Rayleigh Reflection Loss (\rho=2.0, c_p=1600, \alpha=0.0)');
    legend( 'c_s=0', 'c_s=200', 'c_s=400', 'c_s=600' ) ;
end

%%
% Add plot labels common to all tests
%
function add_labels
    grid ; 
    xlabel('Grazing Angle (deg)')
    ylabel('Reflection Loss (dB)')
    set(gca,'Xlim',[0 90]);
    set(gca,'Ylim',[0 10]);
end
